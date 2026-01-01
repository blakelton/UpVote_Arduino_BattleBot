# Feature Plan: AFMotor Library Integration

## Overview

The UpVote battlebot firmware currently uses manual 74HC595 shift register control and direct PWM pin manipulation to control the L293D V1 Motor Shield. This approach is incompatible with the shield's design, which requires the Adafruit Motor Shield V1 library (AFMotor.h). This feature replaces the manual hardware control with proper AFMotor library integration, resolving all motor control issues (constant spinning, erratic behavior, wrong directions).

This is a **critical architectural refactoring** that touches the core actuator layer while preserving all existing functionality in safety, mixing, weapon, servo, and diagnostics modules.

## Requirements

### Functional Requirements
- [ ] **FR-1**: All 4 drive motors controllable via AFMotor library
- [ ] **FR-2**: Motor direction (FORWARD/BACKWARD) responds correctly to signed commands
- [ ] **FR-3**: Motor speed (0-255) responds correctly to command magnitude
- [ ] **FR-4**: Motors stop completely when commanded to zero
- [ ] **FR-5**: Motor polarity inversion flags (`MOTOR_*_INVERTED`) work correctly
- [ ] **FR-6**: Weapon ESC and servo control unaffected by motor changes
- [ ] **FR-7**: Emergency stop functionality preserved
- [ ] **FR-8**: Motor slew-rate limiting preserved (if needed)

### Non-Functional Requirements
- [ ] **NFR-1**: No increase in RAM usage (already constrained at 2KB)
- [ ] **NFR-2**: No impact on 100Hz control loop timing
- [ ] **NFR-3**: All existing safety checks remain functional
- [ ] **NFR-4**: Code maintains existing modular structure
- [ ] **NFR-5**: Pin definitions managed by AFMotor library (no manual pin control)

## Success Criteria
- [ ] **SC-1**: With sticks centered, all 4 motors are stationary (not spinning)
- [ ] **SC-2**: Each motor responds to its assigned stick axis correctly
- [ ] **SC-3**: Motor direction changes work predictably
- [ ] **SC-4**: Motor inversion flags reverse motor direction as expected
- [ ] **SC-5**: Kill switch, failsafe, and emergency stop all work
- [ ] **SC-6**: Weapon and servo control remain operational
- [ ] **SC-7**: No compile errors or warnings
- [ ] **SC-8**: Memory usage stays within limits (RAM < 400 bytes, Flash < 10KB)

## Architectural Analysis

### Existing Components

| Component | File | Purpose | Relevance |
|-----------|------|---------|-----------|
| **actuators.cpp** | [src/actuators.cpp](c:\Projects\UpVote_Arduino_BattleBot\src\actuators.cpp) | Hardware interface for motors, ESC, servo | **CORE CHANGE**: Entire motor control section rewritten |
| **actuators.h** | [include/actuators.h](c:\Projects\UpVote_Arduino_BattleBot\include\actuators.h) | Public API for actuator module | API unchanged, implementation changes |
| **config.h** | [include/config.h](c:\Projects\UpVote_Arduino_BattleBot\include\config.h) | Pin definitions and constants | Remove manual pin defs, AFMotor handles pins |
| **state.h** | [include/state.h](c:\Projects\UpVote_Arduino_BattleBot\include\state.h) | Runtime state structure | No changes needed |
| **mixing.cpp** | [src/mixing.cpp](c:\Projects\UpVote_Arduino_BattleBot\src\mixing.cpp) | Motor mixing logic | No changes (calls unchanged API) |
| **weapon.cpp** | [src/weapon.cpp](c:\Projects\UpVote_Arduino_BattleBot\src\weapon.cpp) | Weapon ESC control | No changes |
| **servo.cpp** | [src/servo.cpp](c:\Projects\UpVote_Arduino_BattleBot\src\servo.cpp) | Servo control | No changes |
| **main.cpp** | [src/main.cpp](c:\Projects\UpVote_Arduino_BattleBot\src\main.cpp) | Main control loop | No changes |

### Patterns to Follow

**Pattern 1: Module Initialization**
- Location: All modules have `*_init()` function called in `setup()`
- Application: AFMotor objects initialized in `actuators_init()`

**Pattern 2: State-Based Updates**
- Location: All modules read from `g_state` and write outputs
- Application: AFMotor reads motor commands from `g_state.output.motor_*_pwm`

**Pattern 3: Safety-First Design**
- Location: All modules initialize to safe states before anything else
- Application: Motors set to `RELEASE` (coasting) or speed=0 before loop starts

**Pattern 4: Constrained Constants**
- Location: [config.h](c:\Projects\UpVote_Arduino_BattleBot\include\config.h) defines all magic numbers
- Application: Keep existing constants like `MOTOR_DUTY_CLAMP_MAX`, remove pin definitions

### Integration Points

| Interface | Location | Connection Method |
|-----------|----------|-------------------|
| **Public API** | [actuators.h](c:\Projects\UpVote_Arduino_BattleBot\include\actuators.h) | No changes - API stays identical |
| **State structure** | [state.h](c:\Projects\UpVote_Arduino_BattleBot\include\state.h) lines 70-76 | Read `motor_*_pwm`, write same fields |
| **Mixing module** | [mixing.cpp](c:\Projects\UpVote_Arduino_BattleBot\src\mixing.cpp) line 137-140 | Calls `actuators_set_motor()` - unchanged |
| **Main loop** | [main.cpp](c:\Projects\UpVote_Arduino_BattleBot\src\main.cpp) line 111 | Calls `actuators_update()` - unchanged |
| **Safety module** | [safety.cpp](c:\Projects\UpVote_Arduino_BattleBot\src\safety.cpp) | May call `actuators_emergency_stop()` - unchanged |

### New Components

| Component | Responsibility | Location |
|-----------|---------------|----------|
| **AFMotor Objects** | Manage L293D shield motors via library | Static objects in `actuators.cpp` |
| **None** | All changes confined to existing actuators module | - |

### Modified Components

| Component | Current State | Changes Needed |
|-----------|---------------|----------------|
| **actuators.cpp** | Manual shift register + PWM control | Replace with AFMotor library calls |
| **config.h** | Manual pin definitions for shift register | Remove `PIN_SR_*` and incorrect motor PWM pins |
| **platformio.ini** | No AFMotor library | Add library dependency |

### Design Decisions

| Decision | Rationale | Alternatives Considered |
|----------|-----------|------------------------|
| **Use AFMotor V1 library** | Hardware documentation explicitly requires it; avoids reinventing complex driver | Continue manual control (failed), write custom driver (unnecessary duplication) |
| **Keep existing API unchanged** | Minimizes changes to calling code; follows Open/Closed Principle | Expose AFMotor objects directly (violates encapsulation) |
| **Handle inversion in `actuators_set_motor()`** | Maintains existing abstraction layer; inversion is a motor-specific property | Handle in mixing (wrong layer), handle in AFMotor update (mixed responsibilities) |
| **Use 64kHz PWM frequency** | AFMotor standard; reduces audible noise from motors | 1kHz (louder), 8kHz (less smooth) |
| **Keep slew-rate limiting optional** | Currently disabled for testing; can re-enable after basic control works | Remove entirely (lose safety feature), force always-on (harder to debug) |

## Implementation Plan

### Phase 1: Library Setup and Build Verification

**Objective**: Add AFMotor library dependency and verify build system works without breaking existing code.

**Tasks**:
1. Add AFMotor library to [platformio.ini](c:\Projects\UpVote_Arduino_BattleBot\platformio.ini):
   ```ini
   lib_deps =
       adafruit/Adafruit Motor Shield library @ ^1.2.5
   ```
   Note: Use V1 library, NOT V2

2. Create minimal test in `actuators.cpp` to verify library import:
   ```cpp
   #include <AFMotor.h>
   // Comment: Library import verification - will expand in Phase 2
   ```

3. Build firmware and verify:
   - No compile errors from library inclusion
   - Flash size impact acceptable (expect +2-3KB)
   - RAM impact acceptable (expect +10-20 bytes for library overhead)

**Dependencies**:
- Requires: None (this is the first phase)
- Enables: Phase 2 (motor object creation)

**Testing Strategy**:
- Build test: `pio run` must succeed
- Size check: Flash < 12KB (currently ~9.4KB), RAM < 420 bytes (currently ~396 bytes)

**Acceptance Criteria**:
- [ ] AFMotor library added to `platformio.ini`
- [ ] `#include <AFMotor.h>` in `actuators.cpp` compiles successfully
- [ ] Firmware builds without errors or warnings
- [ ] Memory usage within acceptable limits

**Risks & Mitigations**:

| Risk | Impact | Mitigation |
|------|--------|------------|
| Library not found in PlatformIO registry | HIGH - blocks all work | Research correct library name; may need manual installation |
| Library increases RAM beyond 2KB limit | HIGH - firmware won't fit | Profile exact RAM usage; optimize if needed |
| Library conflicts with existing timers | MEDIUM - PWM interference | Test early; AFMotor uses Timer0/Timer1 which we're already using |

---

### Phase 2: Motor Object Creation and Initialization

**Objective**: Create AF_DCMotor objects and integrate into `actuators_init()` without removing old code yet.

**Tasks**:
1. Create motor objects in [actuators.cpp](c:\Projects\UpVote_Arduino_BattleBot\src\actuators.cpp) (after includes):
   ```cpp
   // AFMotor library objects for L293D V1 Motor Shield
   // MOTOR12_64KHZ: 64kHz PWM for M1/M2 (uses Timer0)
   // MOTOR34_64KHZ: 64kHz PWM for M3/M4 (uses Timer1)
   static AF_DCMotor motor_rl(1, MOTOR12_64KHZ);  // M1 terminal = Rear-Left
   static AF_DCMotor motor_rr(2, MOTOR12_64KHZ);  // M2 terminal = Rear-Right
   static AF_DCMotor motor_fr(3, MOTOR34_64KHZ);  // M3 terminal = Front-Right
   static AF_DCMotor motor_fl(4, MOTOR34_64KHZ);  // M4 terminal = Front-Left
   ```

2. Create new initialization function (parallel to existing init code):
   ```cpp
   static void init_afmotor_motors() {
     // Initialize all motors to stopped, coasting state
     motor_rl.setSpeed(0);
     motor_rl.run(RELEASE);
     motor_rr.setSpeed(0);
     motor_rr.run(RELEASE);
     motor_fr.setSpeed(0);
     motor_fr.run(RELEASE);
     motor_fl.setSpeed(0);
     motor_fl.run(RELEASE);
   }
   ```

3. Call from `actuators_init()` AFTER existing shift register init (safe fallback):
   ```cpp
   void actuators_init() {
     // OLD CODE - keep for now as fallback
     pinMode(PIN_SR_LATCH, OUTPUT);
     // ... existing code ...

     // NEW CODE - AFMotor initialization
     init_afmotor_motors();

     // Weapon/servo initialization - unchanged
     pinMode(PIN_WEAPON_ESC, OUTPUT);
     // ... rest unchanged ...
   }
   ```

4. Test build and verify objects create successfully

**Dependencies**:
- Requires: Phase 1 complete (library available)
- Enables: Phase 3 (motor control implementation)

**Testing Strategy**:
- Build test: Firmware compiles and links successfully
- Size test: Monitor RAM/Flash increase from object creation
- Hardware test: Upload and verify robot still boots (old code path still active)

**Acceptance Criteria**:
- [ ] 4 `AF_DCMotor` objects created with correct motor numbers (1-4)
- [ ] Objects use correct PWM frequency (64kHz)
- [ ] `init_afmotor_motors()` function created
- [ ] Firmware builds and uploads successfully
- [ ] Robot boots without errors (LED pattern normal)

**Risks & Mitigations**:

| Risk | Impact | Mitigation |
|------|--------|------------|
| Static object construction fails | HIGH - crashes on boot | Test early; add serial debug if needed |
| Timer conflicts with existing PWM | MEDIUM - servo/weapon affected | Verify weapon ESC (D3/Timer2) and servo (D11/Timer2) still work |
| Increased RAM causes stack overflow | HIGH - random crashes | Monitor stack usage carefully; reduce buffer sizes if needed |

---

### Phase 3: AFMotor Update Function Implementation

**Objective**: Implement new motor update logic using AFMotor library while keeping old code active.

**Tasks**:
1. Create new update function in [actuators.cpp](c:\Projects\UpVote_Arduino_BattleBot\src\actuators.cpp):
   ```cpp
   static void update_afmotor_motors() {
     // Read motor commands from global state
     int16_t fl = g_state.output.motor_fl_pwm;
     int16_t fr = g_state.output.motor_fr_pwm;
     int16_t rl = g_state.output.motor_rl_pwm;
     int16_t rr = g_state.output.motor_rr_pwm;

     // Update Rear-Left motor (M1 terminal)
     uint8_t rl_speed = (uint8_t)constrain(abs(rl), 0, 255);
     motor_rl.setSpeed(rl_speed);
     if (rl_speed == 0) {
       motor_rl.run(RELEASE);  // Coast when stopped
     } else {
       motor_rl.run(rl >= 0 ? FORWARD : BACKWARD);
     }

     // Update Rear-Right motor (M2 terminal)
     uint8_t rr_speed = (uint8_t)constrain(abs(rr), 0, 255);
     motor_rr.setSpeed(rr_speed);
     if (rr_speed == 0) {
       motor_rr.run(RELEASE);
     } else {
       motor_rr.run(rr >= 0 ? FORWARD : BACKWARD);
     }

     // Update Front-Right motor (M3 terminal)
     uint8_t fr_speed = (uint8_t)constrain(abs(fr), 0, 255);
     motor_fr.setSpeed(fr_speed);
     if (fr_speed == 0) {
       motor_fr.run(RELEASE);
     } else {
       motor_fr.run(fr >= 0 ? FORWARD : BACKWARD);
     }

     // Update Front-Left motor (M4 terminal)
     uint8_t fl_speed = (uint8_t)constrain(abs(fl), 0, 255);
     motor_fl.setSpeed(fl_speed);
     if (fl_speed == 0) {
       motor_fl.run(RELEASE);
     } else {
       motor_fl.run(fl >= 0 ? FORWARD : BACKWARD);
     }
   }
   ```

2. Add compile-time switch to `actuators_update()`:
   ```cpp
   void actuators_update() {
     // TEMPORARY: Switch between old and new motor control
     #define USE_AFMOTOR 1  // Set to 1 to test AFMotor, 0 for old code

     #if USE_AFMOTOR
       update_afmotor_motors();
     #else
       // OLD CODE - original implementation
       int16_t fl = g_state.output.motor_fl_pwm;
       // ... existing code ...
     #endif

     // Weapon and servo updates - unchanged
     uint16_t weapon_us = g_state.output.weapon_us;
     // ... rest unchanged ...
   }
   ```

3. Build with `USE_AFMOTOR 0` first - verify old code still works
4. Build with `USE_AFMOTOR 1` - test new implementation

**Dependencies**:
- Requires: Phase 2 complete (motor objects created)
- Enables: Phase 4 (hardware testing and cleanup)

**Testing Strategy**:
- Compile test: Both `USE_AFMOTOR 0` and `USE_AFMOTOR 1` build successfully
- Regression test: Upload with `USE_AFMOTOR 0`, verify behavior unchanged
- New path test: Upload with `USE_AFMOTOR 1`, verify motors respond to AFMotor commands

**Acceptance Criteria**:
- [ ] `update_afmotor_motors()` function created
- [ ] All 4 motors updated via AFMotor library
- [ ] Motor direction logic correct (sign check)
- [ ] Motor speed logic correct (absolute value)
- [ ] Zero-speed motors set to `RELEASE` (coasting)
- [ ] Compile-time switch allows testing both implementations

**Risks & Mitigations**:

| Risk | Impact | Mitigation |
|------|--------|------------|
| AFMotor direction reversed | MEDIUM - motors go wrong way | Easy to fix: swap FORWARD/BACKWARD |
| Motor speed scaling wrong | LOW - motors too fast/slow | Verify 0-255 range matches PWM range |
| RELEASE mode causes drift | LOW - robot moves when stopped | Can change to BRAKE if needed |

---

### Phase 4: Hardware Testing and Motor Inversion

**Objective**: Test AFMotor implementation on hardware, fix motor directions, integrate inversion flags.

**Tasks**:
1. **Hardware Test 1: Basic Motor Control**
   - Upload firmware with `USE_AFMOTOR 1`
   - Center all sticks
   - **CRITICAL VERIFICATION**: All motors must be stationary (not spinning)
   - If any motor spins: STOP, debug AFMotor integration

2. **Hardware Test 2: Individual Motor Response**
   - Test each motor independently using direct motor test mode in [mixing.cpp](c:\Projects\UpVote_Arduino_BattleBot\src\mixing.cpp)
   - Verify correct motor-to-terminal mapping:
     - Right stick Y → M1 (Rear-Left)
     - Right stick X → M2 (Rear-Right)
     - Left stick X → M3 (Front-Right)
     - Left stick Y → M4 (Front-Left)
   - Document which motors are spinning backwards

3. **Hardware Test 3: Motor Direction Correction**
   - Update inversion flags in [config.h](c:\Projects\UpVote_Arduino_BattleBot\include\config.h) based on test results
   - Example:
     ```cpp
     #define MOTOR_FL_INVERTED  false  // Update based on hardware test
     #define MOTOR_FR_INVERTED  true   // Invert if motor goes backwards
     #define MOTOR_RL_INVERTED  false
     #define MOTOR_RR_INVERTED  true
     ```

4. **Integrate Inversion Logic**
   - Modify `update_afmotor_motors()` to apply inversion:
     ```cpp
     // Apply inversion flags
     int16_t rl_adjusted = MOTOR_RL_INVERTED ? -rl : rl;
     int16_t rr_adjusted = MOTOR_RR_INVERTED ? -rr : rr;
     int16_t fr_adjusted = MOTOR_FR_INVERTED ? -fr : fr;
     int16_t fl_adjusted = MOTOR_FL_INVERTED ? -fl : fl;

     // Then update motors with adjusted values
     uint8_t rl_speed = (uint8_t)constrain(abs(rl_adjusted), 0, 255);
     motor_rl.setSpeed(rl_speed);
     if (rl_speed == 0) {
       motor_rl.run(RELEASE);
     } else {
       motor_rl.run(rl_adjusted >= 0 ? FORWARD : BACKWARD);
     }
     // Repeat for other motors...
     ```

5. **Hardware Test 4: Verify Inversions**
   - Re-test all 4 motors
   - Confirm all motors now move in correct direction

**Dependencies**:
- Requires: Phase 3 complete (AFMotor update function)
- Enables: Phase 5 (cleanup and final verification)

**Testing Strategy**:
- **Test 1**: Stationary verification (sticks centered)
- **Test 2**: Single-motor control (one stick per motor)
- **Test 3**: Direction verification (forward stick = forward motion)
- **Test 4**: Inversion verification (flags reverse direction)

**Acceptance Criteria**:
- [ ] **SUCCESS CRITERION MET**: All motors stationary with sticks centered
- [ ] Each motor responds to correct stick axis
- [ ] Motor directions match stick input (forward stick = forward motion)
- [ ] Inversion flags correctly reverse motor direction
- [ ] No unexpected motor behavior (erratic spinning, etc.)

**Risks & Mitigations**:

| Risk | Impact | Mitigation |
|------|--------|------------|
| Motor still spins with sticks centered | CRITICAL - AFMotor not working | Check AFMotor library version; verify motor terminal connections |
| Wrong motor responds | MEDIUM - wiring mismatch | Re-verify physical motor-to-terminal wiring |
| All motors inverted | LOW - easy fix | Global sign flip in mixing or actuator code |

---

### Phase 5: Cleanup, Emergency Stop, and Documentation

**Objective**: Remove old shift register code, update emergency stop, finalize implementation.

**Tasks**:
1. **Remove Old Code** from [actuators.cpp](c:\Projects\UpVote_Arduino_BattleBot\src\actuators.cpp):
   - Delete `#define USE_AFMOTOR` switch
   - Delete old motor update code block
   - Delete `shift_register_write()` function (lines 16-21)
   - Delete `set_motor_direction()` function (lines 26-44)
   - Delete `g_shift_reg_state` variable (line 14)
   - Remove shift register initialization from `actuators_init()`:
     ```cpp
     // DELETE THESE LINES:
     pinMode(PIN_SR_LATCH, OUTPUT);
     pinMode(PIN_SR_ENABLE, OUTPUT);
     pinMode(PIN_SR_DATA, OUTPUT);
     pinMode(PIN_SR_CLOCK, OUTPUT);
     digitalWrite(PIN_SR_ENABLE, LOW);
     shift_register_write(0x00);

     pinMode(PIN_MOTOR_FL_PWM, OUTPUT);
     pinMode(PIN_MOTOR_FR_PWM, OUTPUT);
     pinMode(PIN_MOTOR_RL_PWM, OUTPUT);
     pinMode(PIN_MOTOR_RR_PWM, OUTPUT);
     analogWrite(PIN_MOTOR_FL_PWM, SAFE_MOTOR_PWM);
     analogWrite(PIN_MOTOR_FR_PWM, SAFE_MOTOR_PWM);
     analogWrite(PIN_MOTOR_RL_PWM, SAFE_MOTOR_PWM);
     analogWrite(PIN_MOTOR_RR_PWM, SAFE_MOTOR_PWM);
     ```

2. **Remove Pin Definitions** from [config.h](c:\Projects\UpVote_Arduino_BattleBot\include\config.h):
   ```cpp
   // DELETE THESE LINES:
   #define PIN_MOTOR_FL_PWM    9
   #define PIN_MOTOR_FR_PWM   10
   #define PIN_MOTOR_RL_PWM    5
   #define PIN_MOTOR_RR_PWM    6

   #define PIN_SR_LATCH       12
   #define PIN_SR_ENABLE       7
   #define PIN_SR_DATA         8
   #define PIN_SR_CLOCK        4
   ```

   Add comment explaining AFMotor handles pins:
   ```cpp
   // Motor control pins managed internally by AFMotor library
   // L293D V1 Motor Shield uses:
   //   M1 (Rear-Left): D5 (PWM), shift register for direction
   //   M2 (Rear-Right): D6 (PWM), shift register for direction
   //   M3 (Front-Right): D11 (PWM), shift register for direction
   //   M4 (Front-Left): D3 (PWM), shift register for direction
   //   74HC595 shift register: D4 (clock), D7 (enable), D8 (data), D12 (latch)
   ```

3. **Update Emergency Stop** in `actuators_emergency_stop()`:
   ```cpp
   void actuators_emergency_stop() {
     // Stop all motors immediately using AFMotor
     motor_rl.setSpeed(0);
     motor_rl.run(RELEASE);
     motor_rr.setSpeed(0);
     motor_rr.run(RELEASE);
     motor_fr.setSpeed(0);
     motor_fr.run(RELEASE);
     motor_fl.setSpeed(0);
     motor_fl.run(RELEASE);

     // Update state to reflect emergency stop
     g_state.output.motor_fl_pwm = 0;
     g_state.output.motor_fr_pwm = 0;
     g_state.output.motor_rl_pwm = 0;
     g_state.output.motor_rr_pwm = 0;

     // Weapon and servo emergency stop - unchanged
     analogWrite(PIN_WEAPON_ESC, 0);
     analogWrite(PIN_SELFRIGHT_SERVO, 0);
     g_state.output.weapon_us = SAFE_WEAPON_US;
     g_state.output.servo_us = SAFE_SERVO_US;
   }
   ```

4. **Optionally Re-enable Slew-Rate Limiting** in `actuators_set_motor()`:
   - Currently disabled (line 95-97)
   - If desired for thermal protection, re-enable:
     ```cpp
     // Step 3: Apply slew-rate limiting
     int16_t slewed_command = apply_slew_rate(g_motor_previous[motor_index], adjusted_command);
     g_motor_previous[motor_index] = slewed_command;
     ```
   - User decision: Enable now or leave for future tuning?

5. **Update Code Comments**:
   - Add header comment to `actuators.cpp` indicating AFMotor library usage
   - Update comments in `actuators_init()` to reflect new initialization
   - Document motor terminal mapping in comments

6. **Final Build and Memory Check**:
   - Build firmware: `pio run`
   - Verify no warnings
   - Check memory usage:
     - RAM target: < 500 bytes (currently ~396 bytes)
     - Flash target: < 12KB (currently ~9.4KB)

**Dependencies**:
- Requires: Phase 4 complete (hardware tested, inversions correct)
- Enables: Project completion and deployment

**Testing Strategy**:
- Build test: Clean build with no warnings
- Hardware test: Re-verify all functionality still works
- Safety test: Emergency stop test (verify all motors stop immediately)
- Regression test: Verify weapon and servo still functional

**Acceptance Criteria**:
- [ ] All old shift register code removed
- [ ] All manual pin definitions removed
- [ ] Emergency stop uses AFMotor library
- [ ] Code comments updated
- [ ] Firmware builds cleanly (no errors, no warnings)
- [ ] Memory usage within limits
- [ ] All motors work correctly
- [ ] Emergency stop functional
- [ ] Weapon and servo unaffected

**Risks & Mitigations**:

| Risk | Impact | Mitigation |
|------|--------|------------|
| Accidentally break working code | MEDIUM - regression | Test thoroughly after each deletion; keep git commit before cleanup |
| Forget to update emergency stop | HIGH - safety issue | Explicit task and testing for emergency stop |
| Memory usage increases | LOW - still under limits | Already verified in earlier phases |

---

### Phase 6: Holonomic Mixing Re-integration

**Objective**: Transition from direct motor test mode back to full holonomic drive mixing.

**Tasks**:
1. **Current State Assessment**:
   - [mixing.cpp](c:\Projects\UpVote_Arduino_BattleBot\src\mixing.cpp) currently in MOTOR TEST MODE (line 107-141)
   - One stick per motor for individual testing
   - Large deadband (0.2) for safety

2. **Restore Holonomic Mixing**:
   - Replace motor test code with proper holonomic mixing:
     ```cpp
     void mixing_update() {
       // Get stick inputs (deadband already applied in input module)
       float forward = g_state.input.pitch;  // CH2: Forward/back
       float strafe = g_state.input.roll;    // CH1: Strafe left/right
       float rotate = g_state.input.yaw;     // CH4: Rotate left/right

       // Holonomic drive mixing (mecanum/omni wheel kinematics)
       // FL = forward + strafe + rotate
       // FR = forward - strafe - rotate
       // RL = forward - strafe + rotate
       // RR = forward + strafe - rotate

       float fl_mixed = forward + strafe + rotate;
       float fr_mixed = forward - strafe - rotate;
       float rl_mixed = forward - strafe + rotate;
       float rr_mixed = forward + strafe - rotate;

       // Normalize to prevent saturation
       normalize_outputs(&fl_mixed, &fr_mixed, &rl_mixed, &rr_mixed);

       // Apply expo curve for smoother control
       fl_mixed = apply_expo(fl_mixed, g_mode_params.expo);
       fr_mixed = apply_expo(fr_mixed, g_mode_params.expo);
       rl_mixed = apply_expo(rl_mixed, g_mode_params.expo);
       rr_mixed = apply_expo(rr_mixed, g_mode_params.expo);

       // Scale to max duty cycle for current drive mode
       int16_t fl_cmd = (int16_t)(fl_mixed * g_mode_params.max_duty);
       int16_t fr_cmd = (int16_t)(fr_mixed * g_mode_params.max_duty);
       int16_t rl_cmd = (int16_t)(rl_mixed * g_mode_params.max_duty);
       int16_t rr_cmd = (int16_t)(rr_mixed * g_mode_params.max_duty);

       // Apply motor commands
       actuators_set_motor(0, rl_cmd);  // Rear-Left
       actuators_set_motor(1, rr_cmd);  // Rear-Right
       actuators_set_motor(2, fl_cmd);  // Front-Left
       actuators_set_motor(3, fr_cmd);  // Front-Right
     }
     ```

3. **Test Holonomic Drive**:
   - Upload firmware
   - Test forward/backward (right stick Y)
   - Test strafe left/right (right stick X)
   - Test rotate left/right (left stick X)
   - Test combined motions (diagonal, rotate while moving)

4. **Verify Safety Integrations**:
   - Test kill switch (SD): Motors must stop immediately
   - Test failsafe (turn off TX): Motors must stop within 200ms
   - Verify LED diagnostics show correct status

**Dependencies**:
- Requires: Phase 5 complete (code cleaned up)
- Enables: Full system operation

**Testing Strategy**:
- **Test 1**: Forward/backward only (verify correct direction)
- **Test 2**: Strafe left/right only (verify correct direction)
- **Test 3**: Rotate left/right only (verify correct direction)
- **Test 4**: Combined motions (verify smooth blending)
- **Test 5**: Kill switch (verify immediate stop)
- **Test 6**: Failsafe (verify 200ms timeout)

**Acceptance Criteria**:
- [ ] Holonomic mixing code restored
- [ ] Forward/backward motion correct
- [ ] Strafe left/right motion correct
- [ ] Rotation left/right correct
- [ ] Combined motions smooth and predictable
- [ ] Kill switch stops all motors
- [ ] Failsafe stops motors within 200ms
- [ ] Drive mode switching works (Beginner/Normal/Aggressive)

**Risks & Mitigations**:

| Risk | Impact | Mitigation |
|------|--------|------------|
| Mixing math incorrect | MEDIUM - wrong drive behavior | Use proven holonomic mixing formulas; test each axis independently |
| Motor directions wrong in mixing | LOW - easy to fix | Adjust signs in mixing equations or use inversion flags |
| Normalization causes saturation | LOW - reduced control authority | Tune expo and max duty cycle |

---

## Design Principles Applied

### SOLID Application

- **Single Responsibility Principle (SRP)**:
  - Actuators module ONLY handles hardware interface
  - Mixing module ONLY handles kinematics
  - Safety module ONLY handles safety logic
  - AFMotor library ONLY handles L293D shield control
  - Each module has one reason to change

- **Open/Closed Principle (OCP)**:
  - Public API (`actuators_set_motor()`, `actuators_update()`, etc.) unchanged
  - Implementation changes from manual control to AFMotor library
  - Calling code (mixing, weapon, servo) requires zero modifications
  - Open for extension (AFMotor library), closed for modification (API)

- **Liskov Substitution Principle (LSP)**:
  - AFMotor implementation substitutes for manual implementation
  - Behavioral contract preserved: motors respond to signed commands [-255, +255]
  - No unexpected side effects from substitution

- **Interface Segregation Principle (ISP)**:
  - `actuators.h` provides focused interface: init, update, set_motor, emergency_stop
  - Clients only depend on functions they use
  - No bloated interface with unused functions

- **Dependency Inversion Principle (DIP)**:
  - High-level mixing logic depends on abstract "set motor" interface
  - Low-level details (AFMotor vs manual control) hidden behind interface
  - Dependency flows toward abstraction, not implementation

### DRY Considerations

**Duplication ELIMINATED**:
- Old code: 100+ lines of manual shift register control
- New code: AFMotor library handles this internally (DRY at library level)
- Motor update logic: Single pattern repeated 4 times (unavoidable for 4 motors)

**Acceptable Repetition**:
- Each motor's update code is similar but operates on different objects
- Could abstract into helper function, but clarity > minimal repetition for 4 motors
- Trade-off: Slightly verbose code is more readable for embedded debugging

**Future DRY Opportunities**:
- If more motors added, create helper function `update_motor(motor_obj, command, inverted)`

### Complexity Management

**Complexity REDUCED**:
- Before: Manual bit manipulation + PWM coordination + pin management = high cognitive load
- After: Simple `setSpeed()` + `run()` calls = low cognitive load
- Cyclomatic complexity per function: < 5 (well within limit of 10)

**Abstraction Layers**:
1. **Application Layer**: Mixing logic (holonomic drive kinematics)
2. **Interface Layer**: `actuators_set_motor()` (command transformation)
3. **Hardware Layer**: AFMotor library (L293D shield control)
4. **Physical Layer**: L293D H-bridges and motors

**Complexity Metrics**:
- Lines of code: ~180 → ~120 (33% reduction)
- Function count: 7 → 5 (2 fewer functions)
- Max nesting depth: 3 (within limit of 4)
- Cyclomatic complexity: Max 4 per function (within limit of 10)

## Testing Strategy

### Unit Testing
(Manual on-hardware testing - no automated unit tests for embedded firmware)

**actuators.cpp**:
- Test `actuators_init()`: Verify all motors start in stopped state
- Test `actuators_set_motor()`: Verify command transformation (inversion, clamping)
- Test `actuators_update()`: Verify AFMotor library calls with correct parameters
- Test `actuators_emergency_stop()`: Verify all motors stop immediately

### Integration Testing

**Motor → Actuator → AFMotor**:
1. Set motor command via `actuators_set_motor()`
2. Call `actuators_update()`
3. Verify motor spins at correct speed and direction

**Mixing → Actuator**:
1. Set stick inputs in `g_state.input`
2. Call `mixing_update()`
3. Verify motors respond to holonomic mixing (forward, strafe, rotate)

**Safety → Actuator**:
1. Activate kill switch
2. Verify `actuators_update()` receives zero commands
3. Verify motors stop

**Failsafe → Actuator**:
1. Disconnect transmitter
2. Verify link loss detected within 200ms
3. Verify motors stop

### Manual Testing

**Scenario 1: Boot Sequence**:
- Power on robot with battery
- **Verify**: LED shows booting pattern
- **Verify**: All motors stationary (not spinning)
- **Verify**: No unexpected sounds (grinding, whining)

**Scenario 2: Single-Axis Drive**:
- Center all sticks
- Move right stick forward slowly
- **Verify**: All 4 motors spin forward at same speed
- **Verify**: Motor speed proportional to stick displacement
- Center stick
- **Verify**: All motors stop

**Scenario 3: Strafe**:
- Move right stick left
- **Verify**: Robot strafes left (FL/RR forward, FR/RL reverse)
- **Verify**: No forward/backward motion (pure strafe)

**Scenario 4: Rotation**:
- Move left stick left
- **Verify**: Robot rotates counter-clockwise (FL/RR forward, FR/RL forward)
- **Verify**: No translation (pure rotation)

**Scenario 5: Combined Motion**:
- Move right stick diagonally forward-left
- Move left stick left (rotate counter-clockwise)
- **Verify**: Robot moves diagonally while rotating
- **Verify**: Motion smooth and predictable

**Scenario 6: Kill Switch**:
- Drive robot forward at half speed
- Activate kill switch (SD)
- **Verify**: All motors stop within 100ms
- **Verify**: LED shows kill switch active

**Scenario 7: Failsafe**:
- Drive robot forward
- Turn off transmitter
- **Verify**: Motors stop within 200ms
- **Verify**: LED shows link lost pattern

**Scenario 8: Emergency Stop**:
- Drive robot forward
- Call `actuators_emergency_stop()` (via safety module or manual trigger)
- **Verify**: All motors stop immediately
- **Verify**: Weapon stops
- **Verify**: Servo returns to neutral

**Scenario 9: Motor Inversion**:
- Test each motor individually
- Toggle `MOTOR_*_INVERTED` flag
- **Verify**: Motor direction reverses
- **Verify**: No other motors affected

**Scenario 10: Drive Mode Switching**:
- Test in BEGINNER mode (50% duty cycle, high expo)
- **Verify**: Gentle, slow response
- Switch to AGGRESSIVE mode (80% duty cycle, low expo)
- **Verify**: Fast, responsive control

## Risk Assessment

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| AFMotor library not compatible with Arduino UNO | Low | HIGH | Verified library targets Arduino platform; extensive documentation exists |
| Timer conflicts break weapon/servo | Low | MEDIUM | Weapon uses Timer2, servo uses D11; AFMotor uses Timer0/Timer1; no overlap |
| RAM overflow from library overhead | Low | HIGH | Library overhead ~10-20 bytes; current usage 396/2048 bytes (19.3%); plenty of headroom |
| Motor direction reversed | HIGH | LOW | Easy to fix: toggle inversion flags or swap FORWARD/BACKWARD in code |
| AFMotor speed range mismatch | Medium | LOW | AFMotor uses 0-255, our code already uses this range; no scaling needed |
| Shift register state corruption | Low | LOW | AFMotor handles shift register internally; no manual corruption possible |
| Emergency stop doesn't work with AFMotor | Low | HIGH | Test emergency stop early in Phase 4; fallback: use `BRAKE` mode instead of `RELEASE` |
| Holonomic mixing math wrong | Medium | MEDIUM | Use proven mixing formulas; test each axis independently before combining |
| Library increases flash beyond 32KB | Very Low | HIGH | Current usage 9.4KB; library ~2-3KB; total ~12KB; well under 32KB limit |
| Motor wiring actually incorrect | Medium | LOW | Hardware test will reveal; re-label terminals if needed; not a firmware issue |

## Open Questions

1. **Slew-rate limiting**: Should we re-enable slew-rate limiting immediately, or leave disabled for initial testing?
   - **Recommendation**: Leave disabled for Phase 4 hardware testing; re-enable in Phase 6 after holonomic mixing works
   - **Rationale**: Simpler debugging without slew-rate smoothing; can verify raw motor response

2. **Motor stop mode**: Should motors use `RELEASE` (coast) or `BRAKE` when stopped?
   - **Current implementation**: Uses `RELEASE` (coasting)
   - **Alternative**: Use `BRAKE` (active braking) for tighter position holding
   - **Recommendation**: Start with `RELEASE`; switch to `BRAKE` if robot drifts too much
   - **Rationale**: `RELEASE` draws less current; `BRAKE` may cause slight motor heating

3. **Motor terminal verification**: Should we physically verify motor-to-terminal wiring before Phase 4?
   - **Recommendation**: Trust existing wiring notes (M1=RL, M2=RR, M3=FR, M4=FL)
   - **Rationale**: Hardware test in Phase 4 will catch any wiring errors; no need for preemptive disassembly

4. **Drive mode tuning**: Should we re-tune drive mode parameters (expo, max duty) after AFMotor integration?
   - **Current values**:
     - BEGINNER: 50% duty, 0.7 expo
     - NORMAL: 65% duty, 0.4 expo
     - AGGRESSIVE: 80% duty, 0.2 expo
   - **Recommendation**: Keep existing values initially; re-tune based on user feedback after testing
   - **Rationale**: Values were calculated based on mechanical constraints, not control method

5. **Documentation**: Should we create a hardware compatibility document?
   - **Recommendation**: Yes - add `docs/HARDWARE.md` listing:
     - Compatible shields (L293D V1)
     - Required libraries (AFMotor V1)
     - Pin usage documentation
     - Motor terminal mapping
   - **Rationale**: Prevents future confusion; helps other developers

---

**Status**: ready
**Created**: 2025-12-29
**Author**: feature-architect (Claude Sonnet 4.5)
**Estimated Effort**: 6 phases, ~4-6 hours of development + testing
**Next Step**: Orchestrator assigns embedded-developer to implement Phase 1
