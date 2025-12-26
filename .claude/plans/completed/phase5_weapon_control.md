# Feature Plan: Phase 5 - Weapon Control & Arming State Machine

## Overview

Phase 5 implements the **safety-critical weapon arming system** with a robust multi-condition state machine. This is the MOST CRITICAL phase for operator and bystander safety - the weapon must NEVER spin unintentionally under any circumstances, including power cycles, link loss, switch bounces, or software bugs.

**Key principle**: Defense in depth - multiple independent checks ensure weapon safety even if one check fails.

## Pre-Phase Requirements

- [ ] **Phase 4 complete**: Drive system tested and working
- [ ] **Weapon mechanically secured**: Either remove weapon blade OR secure it mechanically before testing
- [ ] **ESC calibration complete**: HW30A ESC calibrated (if needed) per `docs/hardware/esc_calibration.md`
- [ ] **Safety verification**: Complete all Phase 1-4 safety tests

---

## Requirements

### Functional Requirements
- [ ] FR-1: Implement DISARMED ↔ ARMED state machine with explicit state transitions
- [ ] FR-2: **Arming preconditions** (ALL must be true):
  - ARM switch in middle position (SA middle)
  - Weapon throttle ≤ 0.03 (near zero)
  - Link healthy (packet within 200ms)
  - Kill switch inactive (SE down)
  - System not in error state
- [ ] FR-3: **Disarming triggers** (ANY causes immediate disarm):
  - ARM switch not middle
  - Kill switch active
  - Link loss
  - System error
  - Watchdog reset
- [ ] FR-4: Weapon soft-start ramping (slower than drive motors)
- [ ] FR-5: Weapon output scaled by throttle input when armed
- [ ] FR-6: **Switch debouncing** (10ms filter on ARM and KILL)
- [ ] FR-7: **Throttle hysteresis**: Arm at ≤0.03, prevent re-arm until <0.10
- [ ] FR-8: Boot always starts in DISARMED state
- [ ] FR-9: Weapon ESC receives min throttle continuously when disarmed

### Non-Functional Requirements
- [ ] NFR-1: State transitions logged (if RAM permits) or visible via LED
- [ ] NFR-2: SRAM usage <1536 bytes (75% of budget)
- [ ] NFR-3: Arming/disarming response <50ms
- [ ] NFR-4: No weapon glitches during state transitions

## Success Criteria
- [ ] SC-1: 50+ power cycles, weapon NEVER spins during boot
- [ ] SC-2: Cannot arm with throttle >0.03
- [ ] SC-3: Link loss disarms within 200ms (scope verified)
- [ ] SC-4: Kill switch disarms within 50ms
- [ ] SC-5: 100+ arm/disarm cycles without failure
- [ ] SC-6: **Edge case tests pass** (see Test Matrix below)
- [ ] SC-7: Watchdog reset → weapon stays disarmed
- [ ] SC-8: Rapid ARM switch toggling doesn't cause unexpected arming

---

## Architectural Analysis

### Arming State Machine

```
                   ┌──────────────┐
           ┌──────►│   DISARMED   │◄──────┐
           │       └───────┬──────┘       │
           │               │              │
           │               │ ALL:         │
           │               │ • ARM=mid    │
           │               │ • throttle≤0.03
           │               │ • link OK    │
           │               │ • kill=off   │
           │               ▼              │
           │       ┌──────────────┐       │
           │       │    ARMED     │───────┘
           │       └──────────────┘
           │               │
           │               │ ANY:
           │               │ • ARM≠mid
           │               │ • kill=on
           │               │ • link bad
           │               │ • error
           └───────────────┘
```

### Safety Edge Cases Addressed

| Edge Case | Behavior | Mitigation |
|-----------|----------|------------|
| ARM switch bounces | Rapid on/off | 10ms debounce filter |
| Throttle drifts slightly above zero while armed | Remains armed | Hysteresis: arm at ≤0.03, block re-arm until <0.10 |
| Link recovered but ARM still active | Remains disarmed | Must re-request arming (toggle ARM off then on) |
| Kill switch bounced/glitched | Rapid activation | 10ms debounce + treat glitches as kill |
| Rapid arm/disarm cycles | Stress test | Debounce prevents thrashing |
| Power brownout during weapon spin | Reset → unknown state | Watchdog triggers, boot to DISARMED |
| Throttle creeps up slowly while armed | Weapon spins faster | Allowed (operator control) |
| ARM switch left in middle position during boot | Could arm unexpectedly | Require ARM toggle off then on to arm |

---

## Implementation Plan

### Phase 5.1: Arming State Machine Core

**Objective**: Implement state machine with debouncing

#### Tasks

1. **Add arming state to RuntimeState** (`include/state.h`):
   ```cpp
   enum ArmState {
     DISARMED = 0,
     ARMED = 1
   };

   struct SafetyState {
     ArmState  arm_state;
     bool      arm_requested_prev;     // For edge detection
     uint32_t  arm_switch_stable_ms;   // Debounce timer
     uint32_t  kill_switch_stable_ms;  // Debounce timer
     bool      arm_switch_debounced;
     bool      kill_switch_debounced;
     float     last_arm_throttle;      // For hysteresis
   };

   struct RuntimeState {
     // ... existing ...
     SafetyState safety;
   };
   ```

2. **Implement debouncing** (`src/safety.cpp`):
   ```cpp
   #define SWITCH_DEBOUNCE_MS 10
   #define ARM_THROTTLE_THRESHOLD 0.03f
   #define REARM_THROTTLE_THRESHOLD 0.10f

   static void safety_update_switch_debounce() {
     extern RuntimeState g_state;
     uint32_t now = millis();

     // Debounce ARM switch
     bool arm_raw = g_state.input.arm_request;
     if (arm_raw != g_state.safety.arm_switch_debounced) {
       // State changed, start/reset timer
       g_state.safety.arm_switch_stable_ms = now;
     } else {
       // State stable
       if ((now - g_state.safety.arm_switch_stable_ms) >= SWITCH_DEBOUNCE_MS) {
         g_state.safety.arm_switch_debounced = arm_raw;
       }
     }

     // Debounce KILL switch
     bool kill_raw = g_state.input.kill;
     if (kill_raw != g_state.safety.kill_switch_debounced) {
       g_state.safety.kill_switch_stable_ms = now;
     } else {
       if ((now - g_state.safety.kill_switch_stable_ms) >= SWITCH_DEBOUNCE_MS) {
         g_state.safety.kill_switch_debounced = kill_raw;
       }
     }
   }
   ```

3. **Implement arming logic with hysteresis**:
   ```cpp
   void safety_update_arming() {
     extern RuntimeState g_state;

     safety_update_switch_debounce();

     bool arm_switch = g_state.safety.arm_switch_debounced;
     bool kill_active = g_state.safety.kill_switch_debounced;
     bool link_ok = safety_is_link_ok();
     float throttle = g_state.input.weapon;
     SystemError error = safety_get_error();

     // Check disarming conditions first (highest priority)
     bool should_disarm = false;

     if (!arm_switch) should_disarm = true;      // ARM switch off
     if (kill_active) should_disarm = true;      // Kill active
     if (!link_ok) should_disarm = true;         // Link loss
     if (error != ERR_NONE) should_disarm = true; // System error

     if (should_disarm) {
       g_state.safety.arm_state = DISARMED;
       g_state.safety.last_arm_throttle = 0.0f;
       return; // Exit early
     }

     // Check arming conditions (only if currently disarmed)
     if (g_state.safety.arm_state == DISARMED) {
       // Throttle hysteresis: if we recently disarmed with high throttle,
       // require throttle to drop below REARM threshold before allowing arm
       bool throttle_ok = false;
       if (g_state.safety.last_arm_throttle > ARM_THROTTLE_THRESHOLD) {
         // Previously had high throttle, need to go lower
         throttle_ok = (throttle < REARM_THROTTLE_THRESHOLD);
       } else {
         // Normal arming
         throttle_ok = (throttle <= ARM_THROTTLE_THRESHOLD);
       }

       if (arm_switch && link_ok && !kill_active &&
           throttle_ok && error == ERR_NONE) {
         // All conditions met, can arm
         g_state.safety.arm_state = ARMED;
       }
     }

     // Update last throttle for hysteresis
     if (g_state.safety.arm_state == DISARMED) {
       g_state.safety.last_arm_throttle = throttle;
     }
   }
   ```

#### Dependencies
- Requires: Phase 4 complete
- Enables: Safe weapon arming

#### Testing Strategy
- **State transition test**: Verify all transitions in state machine
- **Debounce test**: Rapid switch toggling, verify stable response
- **Hysteresis test**: Arm with throttle high, verify block until low

#### Acceptance Criteria
- [ ] State machine transitions correctly
- [ ] Debouncing prevents false triggers
- [ ] Hysteresis prevents immediate re-arm with high throttle

---

### Phase 5.2: Weapon ESC Control

**Objective**: Output weapon throttle to HW30A ESC

#### Tasks

1. **Update actuators module** (`src/actuators.cpp`):
   ```cpp
   #include <Servo.h>

   Servo weapon_esc;

   void actuators_init() {
     // ... existing motor init ...

     // Initialize weapon ESC
     weapon_esc.attach(PIN_WEAPON_ESC);
     weapon_esc.writeMicroseconds(WEAPON_ESC_MIN_US); // Min throttle

     // Wait for ESC to initialize (beep sequence)
     delay(2000);

     actuators_set_safe_state();
   }

   void actuators_set_weapon(float throttle_cmd) {
     // throttle_cmd: 0.0 (min) to 1.0 (max)
     // Clamp
     if (throttle_cmd > 1.0f) throttle_cmd = 1.0f;
     if (throttle_cmd < 0.0f) throttle_cmd = 0.0f;

     // Map to ESC pulse width
     uint16_t pulse_us = WEAPON_ESC_MIN_US +
                         (uint16_t)(throttle_cmd * (WEAPON_ESC_MAX_US - WEAPON_ESC_MIN_US));

     weapon_esc.writeMicroseconds(pulse_us);
   }

   void actuators_set_safe_state() {
     // ... existing motor safe states ...

     // Weapon to minimum throttle
     actuators_set_weapon(0.0f);
   }
   ```

2. **Add weapon ramping** (slower than drive):
   ```cpp
   #define WEAPON_RAMP_RATE 0.01f  // Per loop iteration (slower than drive)

   static float weapon_output = 0.0f;

   void actuators_update_weapon(float weapon_cmd) {
     // Apply ramping
     float delta = weapon_cmd - weapon_output;
     if (delta > WEAPON_RAMP_RATE) delta = WEAPON_RAMP_RATE;
     if (delta < -WEAPON_RAMP_RATE) delta = -WEAPON_RAMP_RATE;

     weapon_output += delta;

     // Clamp
     if (weapon_output > 1.0f) weapon_output = 1.0f;
     if (weapon_output < 0.0f) weapon_output = 0.0f;

     actuators_set_weapon(weapon_output);
   }
   ```

#### Dependencies
- Requires: Phase 5.1 complete
- Enables: Weapon motor control

#### Testing Strategy
- **ESC initialization**: Verify beep sequence after power-on
- **Throttle response**: Verify weapon spins (bench test, NO BLADE!)
- **Ramping**: Verify gradual spin-up
- **Min throttle**: Verify weapon stops at zero

#### Acceptance Criteria
- [ ] ESC initializes correctly
- [ ] Weapon responds to throttle
- [ ] Ramping smooths spin-up
- [ ] Weapon stops cleanly

---

### Phase 5.3: Integration & Edge Case Testing

**Objective**: Verify all safety edge cases

#### State Transition Test Matrix

| Current State | Event | Expected Next State | Test Method |
|---------------|-------|---------------------|-------------|
| DISARMED | ARM=mid, throttle=0, link=ok, kill=off | ARMED | Manual switch activation |
| DISARMED | ARM=mid, throttle>0.03 | DISARMED | Move throttle high, try to arm |
| ARMED | ARM=off | DISARMED | Toggle ARM off |
| ARMED | kill=on | DISARMED | Hit kill switch |
| ARMED | link=bad | DISARMED | Power off TX, measure timing |
| DISARMED | Rapid ARM toggle | DISARMED (stable) | Rapid switch flipping |
| ARMED | Watchdog reset | DISARMED (after reboot) | Trigger watchdog |
| DISARMED | ARM=mid at boot | DISARMED | Power on with ARM mid |
| DISARMED → ARMED → DISARMED (throttle high) | ARM=mid again | DISARMED | Verify hysteresis blocks |

#### Tasks

1. **Implement test harness** (temporary debug code):
   ```cpp
   void test_arming_state_machine() {
     // Print state via Serial (debug build only)
     Serial.print("ARM:");
     Serial.print(g_state.safety.arm_state);
     Serial.print(" SW:");
     Serial.print(g_state.safety.arm_switch_debounced);
     Serial.print(" THR:");
     Serial.println(g_state.input.weapon, 3);
   }
   ```

2. **Execute test matrix**: Manually execute each test case

3. **Endurance test**: 100+ arm/disarm cycles

#### Acceptance Criteria
- [ ] All test matrix cases pass
- [ ] No unexpected state transitions
- [ ] 100+ cycles without failure

---

### Phase 5.4: Main Loop Integration

**Objective**: Integrate weapon control into main control loop

#### Tasks

1. **Update main loop** (`src/main.cpp`):
   ```cpp
   void control_loop_tick() {
     // Read input
     input_update();

     // Update safety (includes arming logic)
     safety_update_arming();

     // Check overall safety
     bool link_ok = safety_is_link_ok();
     bool safe_to_operate = (link_ok && safety_get_error() == ERR_NONE);

     if (!safe_to_operate) {
       // FAILSAFE
       actuators_set_safe_state();
     } else {
       // Normal operation
       if (safety_is_armed()) {
         // Armed: Drive + weapon
         // (Drive control from Phase 4)
         actuators_update_motors(...);
         actuators_update_weapon(g_state.input.weapon);
       } else {
         // Disarmed: Drive only, weapon safe
         actuators_update_motors(...);
         actuators_update_weapon(0.0f);
       }
     }

     diagnostics_update();
     safety_pet_watchdog();
   }
   ```

2. **Update LED diagnostics** to show ARMED state:
   ```cpp
   // In diagnostics.cpp
   // STATUS_SAFE: Slow blink (1Hz)
   // STATUS_ARMED: Fast blink (5Hz)
   ```

#### Acceptance Criteria
- [ ] Weapon only spins when armed
- [ ] LED indicates armed state
- [ ] Failsafe overrides arming

---

## Testing Strategy

### Unit Testing
- **Debounce logic**: Simulate rapid switch changes
- **Hysteresis logic**: Test throttle threshold edge cases
- **State machine**: Test all transitions

### Integration Testing
- **Full arming flow**: TX16S → ELRS → Arduino → weapon ESC
- **Failsafe integration**: Link loss while armed
- **Error integration**: System error while armed

### Safety Verification Testing

**CRITICAL TESTS** (must ALL pass):

1. **Boot Safety Test** (50 cycles):
   - Power on with ARM in various positions
   - Weapon MUST be min throttle every time

2. **Throttle Interlock Test**:
   - Try to arm with throttle at 10%, 25%, 50%, 100%
   - Arming MUST be blocked

3. **Link Loss Test** (10 cycles):
   - Arm weapon, spin to 50%
   - Power off TX
   - Weapon MUST stop within 200ms (scope verify)

4. **Kill Switch Test** (10 cycles):
   - Arm weapon, spin to 50%
   - Hit kill switch
   - Weapon MUST stop within 50ms (scope verify)

5. **Watchdog Test**:
   - Arm weapon
   - Trigger watchdog reset (infinite loop)
   - After reboot, weapon MUST be min throttle

6. **Rapid Toggle Test**:
   - Rapidly toggle ARM switch 20 times
   - System MUST remain stable, no crashes

7. **Hysteresis Test**:
   - Arm with throttle at 50% (blocked)
   - Lower throttle to 5% (still blocked)
   - Lower throttle to 2% (now can arm)

---

## Risk Assessment

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Weapon spins on unexpected state | Low | CRITICAL | Multi-layer checks; extensive testing |
| Switch bounce causes arming glitch | Low | High | 10ms debounce; test with rapid toggles |
| Hysteresis threshold too tight | Medium | Medium | Configurable threshold; tune during testing |
| ESC initialization unreliable | Low | Medium | 2s delay; document ESC beep sequence |
| Weapon doesn't stop on failsafe | Low | CRITICAL | Test link loss 10+ times; scope verify timing |

---

## Open Questions

- [ ] Should arming require explicit toggle (off→on) vs continuous middle? (Recommend: toggle)
- [ ] Should we add arming delay (e.g., must hold ARM for 1s)? (Optional safety margin)
- [ ] Should we log arm/disarm events? (RAM cost vs. diagnostics)

---

**Status**: ready
**Created**: 2025-12-25
**Author**: feature-architect (updated with gap analysis - safety edge cases)
**Updates**: Complete state machine with debouncing, hysteresis, comprehensive test matrix
