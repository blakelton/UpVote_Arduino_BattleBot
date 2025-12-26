# Feature Plan: Phase 1 - Project Setup & Safety Scaffolding

## Overview

Phase 1 establishes the foundational safety infrastructure and modular code architecture for the UpVote battlebot firmware. This phase creates a safe baseline where **nothing moves unintentionally** - all drive motors, weapon, and servo remain in safe states on boot and during all error conditions. The modular structure prevents future code from becoming a monolithic blob while enabling systematic testing of each subsequent phase.

This is the most critical phase because it establishes safety defaults that protect against all undefined states throughout development.

## Pre-Phase Requirements (CRITICAL - Must Complete First!)

**BEFORE writing any code**, complete these hardware verification tasks:

### Hardware Verification Checklist
- [ ] **Identify L293D Shield Model**: Document exact shield variant (Adafruit Motor Shield v1/v2, DFRobot, generic Chinese clone)
- [ ] **Physical Pin Inspection**: Note which Arduino pins the shield uses (check shield documentation + physical traces)
- [ ] **Timer Conflict Check**: Verify Arduino UNO timer assignments:
  ```
  Timer0: Used by delay(), millis(), micros() - Avoid PWM on pins 5, 6
  Timer1: 16-bit, used by Servo library - Pins 9, 10
  Timer2: 8-bit, available - Pins 3, 11
  ```
- [ ] **Create Pin Assignment Document**: Document in `docs/hardware/pin_assignments.md`
- [ ] **Verify No Conflicts**: Ensure motor PWM pins don't conflict with weapon ESC + servo

### Recommended Pin Strategy (verify against your shield!)
```cpp
// RECOMMENDED (assuming generic L293D shield):
// Motors: Use Timer2 pins (3, 11) where possible
// Weapon ESC: Pin 9 (Timer1, Servo library compatible)
// Servo: Pin 10 (Timer1, Servo library compatible)
// Status LED: Pin 13 (built-in, Timer0 but digital only)
```

**Estimated Time**: 1-2 hours
**Deliverable**: `docs/hardware/pin_assignments.md` with verified pinout

---

## Requirements

### Functional Requirements
- [ ] FR-1: All motor outputs default to safe/stopped state on boot
- [ ] FR-2: Weapon ESC receives minimum throttle signal continuously from boot
- [ ] FR-3: Servo positioned at safe neutral angle on boot
- [ ] FR-4: Fixed-rate control loop runs at 100 Hz
- [ ] FR-5: Status LED indicates SAFE/DISARMED state
- [ ] FR-6: Modular code structure isolates config, state, safety, actuators, diagnostics
- [ ] **FR-7: AVR hardware watchdog timer enabled (1s timeout)** ⭐ NEW
- [ ] **FR-8: Error state tracking with LED diagnostic codes** ⭐ NEW
- [ ] **FR-9: Memory usage tracking in build output** ⭐ NEW

### Non-Functional Requirements
- [ ] NFR-1: Code compiles without warnings on PlatformIO
- [ ] **NFR-2: SRAM usage <512 bytes (25% of 2KB budget)** ⭐ UPDATED
- [ ] NFR-3: Control loop timing measured and verified <10ms
- [ ] NFR-4: Module boundaries clear with defined interfaces
- [ ] NFR-5: No dynamic memory allocation
- [ ] **NFR-6: Build configured with `-Wl,--print-memory-usage` flag** ⭐ NEW

## Success Criteria
- [ ] SC-1: 20+ consecutive power cycles show identical safe boot behavior
- [ ] SC-2: Oscilloscope/multimeter confirms all outputs safe (motors stopped, weapon at min throttle)
- [ ] SC-3: Status LED visible and indicates correct state
- [ ] SC-4: Code passes embedded-quality-evaluator with no CRITICAL issues
- [ ] SC-5: SRAM usage <1KB, flash usage <8KB

## Architectural Analysis

### Existing Components
**Current state**: Only boilerplate Arduino main.cpp exists with empty setup() and loop().

**Analysis**: No existing code to integrate with. This phase creates the architectural foundation.

### New Components

All components created in this phase:

| Component | Responsibility | Location |
|-----------|---------------|----------|
| config | Compile-time constants, calibration values, pin definitions | `include/config.h`, `src/config.cpp` |
| state | Runtime state structures (inputs, outputs, timers, flags) | `include/state.h`, `src/state.cpp` |
| safety | Safe initialization, failsafe defaults, watchdog skeleton | `include/safety.h`, `src/safety.cpp` |
| actuators | Motor, ESC, servo output abstractions (stubs) | `include/actuators.h`, `src/actuators.cpp` |
| diagnostics | LED control, optional serial debug | `include/diagnostics.h`, `src/diagnostics.cpp` |

### Integration Points
- **main.cpp**: Orchestrates module initialization and control loop
- **RuntimeState structure**: Central data structure all modules read/write
- **Hardware pins**: L293D shield pins, weapon ESC pin, servo pin, status LED pin

### Design Decisions

| Decision | Rationale | Alternatives Considered |
|----------|-----------|------------------------|
| Modular structure from day 1 | Prevents code from becoming unmaintainable; enables isolated testing | Single-file approach (rejected: becomes unmanageable) |
| Fixed 100 Hz loop | Predictable timing; adequate for human control; manageable on UNO | Event-driven (rejected: harder to reason about); faster rate (rejected: unnecessary CPU load) |
| Static allocation only | UNO has only 2KB RAM; predictable memory footprint | Dynamic allocation (rejected: risk of fragmentation and OOM) |
| Initialize outputs before inputs | Safety critical; outputs must be safe before any other code runs | Initialize in order of file (rejected: not safety-focused) |
| Central RuntimeState structure | Single source of truth; prevents hidden coupling | Global variables (rejected: harder to reason about); message passing (rejected: overkill for UNO) |

## Implementation Plan

### Phase 1.1: Project Structure & Build System
**Objective**: Create modular file structure and verify build system works

#### Tasks
1. Create directory structure:
   ```
   include/config.h
   include/state.h
   include/safety.h
   include/actuators.h
   include/diagnostics.h
   src/config.cpp
   src/state.cpp
   src/safety.cpp
   src/actuators.cpp
   src/diagnostics.cpp
   src/main.cpp (modify existing)
   ```
2. Add include guards to all headers
3. Create minimal stub functions in each module
4. Verify compilation succeeds
5. Document SRAM/flash usage from build output

#### Dependencies
- Requires: PlatformIO project already configured
- Enables: Modular development for all future phases

#### Testing Strategy
- Build test: `pio run` completes without errors or warnings
- Size test: Check SRAM < 1KB, flash < 8KB

#### Acceptance Criteria
- [ ] All files created with proper include guards
- [ ] Build succeeds with zero warnings
- [ ] Memory usage documented

#### Risks & Mitigations
| Risk | Impact | Mitigation |
|------|--------|------------|
| Build configuration issues | High | Verify platformio.ini settings early |
| Include path problems | Medium | Use relative includes, test immediately |

---

### Phase 1.2: Data Structures & Configuration
**Objective**: Define core data structures and pin configuration

#### Tasks
1. In `include/state.h`, define:
   ```cpp
   struct RuntimeState {
     // Input state (zeroed in Phase 2)
     // Safety state
     uint32_t last_rx_ms;
     bool     kill_active;
     // Output state
     uint32_t loop_count;
   };
   extern RuntimeState g_state;
   ```

2. In `include/config.h`, define:
   ```cpp
   // Pin assignments for L293D shield
   #define PIN_MOTOR_FL_PWM    3
   #define PIN_MOTOR_FL_DIR    12
   #define PIN_MOTOR_FR_PWM    11
   #define PIN_MOTOR_FR_DIR    13
   // ... (all 4 motors)

   #define PIN_WEAPON_ESC      9
   #define PIN_SERVO           10
   #define PIN_STATUS_LED      LED_BUILTIN

   // Timing
   #define LOOP_RATE_HZ        100
   #define LOOP_PERIOD_MS      (1000 / LOOP_RATE_HZ)

   // ESC constants
   #define WEAPON_ESC_MIN_US   1000
   #define WEAPON_ESC_MAX_US   2000

   // Servo constants
   #define SERVO_NEUTRAL_US    1500
   ```

3. In `src/state.cpp`, instantiate global state:
   ```cpp
   RuntimeState g_state = {};
   ```

#### Dependencies
- Requires: Phase 1.1 complete
- Enables: All other modules can reference these structures

#### Testing Strategy
- Compilation test: Verify structures compile
- Size test: Monitor SRAM usage

#### Acceptance Criteria
- [ ] RuntimeState structure defined and documented
- [ ] All pin assignments documented
- [ ] Constants defined for all hardware parameters

---

### Phase 1.3: Safe Actuator Initialization
**Objective**: Initialize all outputs to safe states

#### Tasks
1. In `include/actuators.h`, declare:
   ```cpp
   void actuators_init();
   void actuators_update();
   void actuators_set_safe_state();
   ```

2. In `src/actuators.cpp`, implement:
   ```cpp
   void actuators_init() {
     // Set all motor pins as outputs
     pinMode(PIN_MOTOR_FL_PWM, OUTPUT);
     pinMode(PIN_MOTOR_FL_DIR, OUTPUT);
     // ... (all pins)

     // Initialize weapon ESC output (attach and set min throttle)
     // Initialize servo (attach and set neutral)

     // Call safe state immediately
     actuators_set_safe_state();
   }

   void actuators_set_safe_state() {
     // Set all motor PWM to 0
     analogWrite(PIN_MOTOR_FL_PWM, 0);
     // ... (all motors)

     // Set weapon to minimum throttle
     // (use Servo library or manual PWM)

     // Set servo to neutral
   }
   ```

3. In `src/main.cpp`, call `actuators_init()` FIRST in `setup()`:
   ```cpp
   void setup() {
     // CRITICAL: Initialize outputs before anything else
     actuators_init();

     // Then initialize other modules
     diagnostics_init();
     // ...
   }
   ```

#### Dependencies
- Requires: Phase 1.2 complete (pin definitions)
- Enables: Guaranteed safe boot behavior

#### Testing Strategy
- Bench test: Power UNO, measure all output pins with multimeter/scope
- Visual test: Verify motors don't twitch, weapon ESC receives min throttle
- Power cycle test: 10+ cycles, verify consistent behavior

#### Acceptance Criteria
- [ ] All motor PWM outputs read 0V
- [ ] Weapon ESC pin shows 1000µs pulses (min throttle)
- [ ] Servo at neutral position
- [ ] No transient spikes on boot (scope verification)

#### Risks & Mitigations
| Risk | Impact | Mitigation |
|------|--------|------------|
| Weapon ESC interprets init as throttle | Critical | Use Servo library with explicit min throttle; verify with scope |
| Motor shield pins conflict with servo | Medium | Verify L293D shield timer usage doesn't conflict |
| Brownout on servo init | Medium | Initialize servo last; use separate BEC for servo |

---

### Phase 1.4: Control Loop Timing
**Objective**: Implement fixed-rate 100 Hz control loop

#### Tasks
1. In `src/main.cpp`, implement timing:
   ```cpp
   void loop() {
     static uint32_t next_loop_ms = 0;

     uint32_t now = millis();
     if (now >= next_loop_ms) {
       next_loop_ms = now + LOOP_PERIOD_MS;

       // Control loop body
       control_loop_tick();

       g_state.loop_count++;
     }
   }

   void control_loop_tick() {
     // Phase 1: Just maintain safe state
     actuators_set_safe_state();
     diagnostics_update();
   }
   ```

2. Add timing measurement (optional debug):
   ```cpp
   uint32_t loop_start = micros();
   control_loop_tick();
   uint32_t loop_duration = micros() - loop_start;
   // Log if > 5000µs (warning threshold)
   ```

#### Dependencies
- Requires: Phase 1.3 complete
- Enables: Predictable timing for future phases

#### Testing Strategy
- Timing test: Use serial output or LED blink to verify 100 Hz rate
- Performance test: Measure loop execution time (should be <1ms in Phase 1)

#### Acceptance Criteria
- [ ] Loop runs at 100 Hz ±5%
- [ ] Loop execution time <1ms
- [ ] No timing drift over 60 seconds

---

### Phase 1.5: Diagnostics & Enhanced LED Patterns
**Objective**: Visual indication of system state and error codes (replaces Serial debug)

**Design Philosophy**: LED patterns are the PRIMARY diagnostic tool throughout all phases. No UART conflict, works during competition, visible at a glance.

#### Tasks

1. **Define comprehensive diagnostic patterns** (`include/diagnostics.h`):
   ```cpp
   enum SystemStatus {
     STATUS_SAFE,        // Slow blink (1 Hz) - System OK, disarmed
     STATUS_ARMED,       // Fast blink (5 Hz) - Weapon armed
     STATUS_FAILSAFE,    // Solid ON - Link loss or kill active
     STATUS_ERROR        // Error code blink sequence
   };

   void diagnostics_init();
   void diagnostics_update();
   void diagnostics_set_status(SystemStatus status);
   int diagnostics_get_free_ram();  // For telemetry (Phase 2+)
   ```

2. **Implement LED state machine** (`src/diagnostics.cpp`):
   ```cpp
   static SystemStatus current_status = STATUS_SAFE;
   static uint32_t last_blink = 0;
   static bool led_state = false;
   static uint8_t error_blink_count = 0;
   static uint8_t error_blink_phase = 0;

   void diagnostics_init() {
     pinMode(PIN_STATUS_LED, OUTPUT);
     digitalWrite(PIN_STATUS_LED, LOW);
   }

   void diagnostics_update() {
     extern RuntimeState g_state;
     uint32_t now = millis();

     // Determine current status
     SystemError error = safety_get_error();
     if (error != ERR_NONE) {
       current_status = STATUS_ERROR;
     } else if (!safety_is_link_ok()) {
       current_status = STATUS_FAILSAFE;
     } else if (safety_is_armed()) {
       current_status = STATUS_ARMED;
     } else {
       current_status = STATUS_SAFE;
     }

     // Execute blink pattern
     switch (current_status) {
       case STATUS_SAFE:
         // Slow blink: 1 Hz (500ms on, 500ms off)
         if (now - last_blink >= 500) {
           last_blink = now;
           led_state = !led_state;
           digitalWrite(PIN_STATUS_LED, led_state);
         }
         break;

       case STATUS_ARMED:
         // Fast blink: 5 Hz (100ms on, 100ms off)
         if (now - last_blink >= 100) {
           last_blink = now;
           led_state = !led_state;
           digitalWrite(PIN_STATUS_LED, led_state);
         }
         break;

       case STATUS_FAILSAFE:
         // Solid ON
         digitalWrite(PIN_STATUS_LED, HIGH);
         break;

       case STATUS_ERROR:
         // Blink error code: N blinks, pause, repeat
         // ERR_LOOP_OVERRUN=1, ERR_WATCHDOG_RESET=2, etc.
         diagnostics_blink_error_code(error);
         break;
     }
   }

   void diagnostics_blink_error_code(SystemError error) {
     static SystemError last_error = ERR_NONE;  // Track error changes
     uint32_t now = millis();
     uint8_t blink_count = (uint8_t)error;  // Error enum value = blink count

     // Reset state if error changed mid-sequence (prevents confusing mixed patterns)
     if (error != last_error) {
       error_blink_phase = 0;
       error_blink_count = 0;
       last_blink = now;
       led_state = false;
       digitalWrite(PIN_STATUS_LED, LOW);
       last_error = error;
     }

     // State machine for error code blinking
     // Phase 0: Blink N times (100ms on, 100ms off)
     // Phase 1: Long pause (1000ms)
     // Repeat

     if (error_blink_phase == 0) {
       // Blinking phase
       if (now - last_blink >= 100) {
         last_blink = now;
         if (led_state) {
           // Was ON, turn OFF
           digitalWrite(PIN_STATUS_LED, LOW);
           led_state = false;
           error_blink_count++;
           if (error_blink_count >= blink_count) {
             // Done blinking, enter pause
             error_blink_phase = 1;
             error_blink_count = 0;
           }
         } else {
           // Was OFF, turn ON
           digitalWrite(PIN_STATUS_LED, HIGH);
           led_state = true;
         }
       }
     } else {
       // Pause phase
       digitalWrite(PIN_STATUS_LED, LOW);
       if (now - last_blink >= 1000) {
         // Pause done, restart blinking
         error_blink_phase = 0;
         last_blink = now;
       }
     }
   }
   ```

3. **Add free RAM helper** (for future CRSF telemetry):
   ```cpp
   int diagnostics_get_free_ram() {
     extern int __heap_start, *__brkval;
     int v;
     return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
   }
   ```

4. **Update Open Questions** section to document LED patterns:
   - Document all LED patterns in operator manual (Phase 7)
   - STATUS_SAFE: 1 Hz slow blink
   - STATUS_ARMED: 5 Hz fast blink
   - STATUS_FAILSAFE: Solid ON
   - STATUS_ERROR: Blink N times (N = error code)

#### LED Pattern Reference
| State | Pattern | Meaning |
|-------|---------|---------|
| SAFE | ▁▔▁▔ (1 Hz) | System OK, disarmed, ready |
| ARMED | ▁▔▁▔▁▔▁▔ (5 Hz) | Weapon armed - DANGER |
| FAILSAFE | ▔▔▔▔ (Solid) | Link loss or kill switch active |
| ERROR (1 blink) | ▔▁pause▔▁pause | Loop overrun error |
| ERROR (2 blinks) | ▔▁▔▁pause▔▁▔▁ | Watchdog reset error |
| ERROR (3 blinks) | ▔▁▔▁▔▁pause | CRSF timeout (Phase 2+) |

#### Dependencies
- Requires: Phase 1.4 complete
- Enables: Visual diagnostics throughout all phases (no UART needed!)

#### Testing Strategy
- **Visual test**: Verify all 4 patterns (SAFE, ARMED, FAILSAFE, ERROR)
- **Error code test**: Trigger each error, count LED blinks
- **State transition**: Change states, verify pattern updates within 100ms
- **Visibility test**: LED visible from 3 meters in bright light

#### Acceptance Criteria
- [ ] All 4 status patterns implemented
- [ ] Error codes blink correct number of times
- [ ] LED visible and not obscured by hardware
- [ ] Patterns distinguish easily (slow vs fast vs solid vs error)
- [ ] Free RAM helper function works (verify against build output)

---

### Phase 1.6: Safety Module with Watchdog & Error Handling ⭐ UPDATED
**Objective**: Implement hardware watchdog timer and error state tracking

#### Tasks

1. **Update platformio.ini** to enable memory tracking:
   ```ini
   [env:uno]
   platform = atmelavr
   board = uno
   framework = arduino
   build_flags =
     -Wl,--print-memory-usage
   monitor_speed = 115200
   ```

2. In `include/safety.h`, declare:
   ```cpp
   #include <avr/wdt.h>

   // Error codes
   enum SystemError {
     ERR_NONE = 0,
     ERR_LOOP_OVERRUN,      // Control loop took >10ms
     ERR_WATCHDOG_RESET,    // System recovered from watchdog
     // More errors added in later phases
   };

   void safety_init();
   void safety_update();
   void safety_pet_watchdog();
   bool safety_is_armed();
   bool safety_is_link_ok();
   SystemError safety_get_error();
   void safety_set_error(SystemError err);
   ```

3. In `src/safety.cpp`, implement:
   ```cpp
   #include <avr/wdt.h>

   static SystemError current_error = ERR_NONE;

   void safety_init() {
     // Check if this boot was caused by watchdog
     uint8_t mcusr_copy = MCUSR;
     MCUSR = 0;  // Clear reset flags

     if (mcusr_copy & (1 << WDRF)) {
       // Watchdog caused reset!
       current_error = ERR_WATCHDOG_RESET;
     }

     // Enable watchdog timer (1 second timeout)
     wdt_enable(WDTO_1S);
   }

   void safety_pet_watchdog() {
     wdt_reset();  // Pet the watchdog
   }

   void safety_update() {
     // Phase 1: Just pet watchdog, check for loop overruns
     // Future phases: Check link health, arming conditions
   }

   bool safety_is_armed() {
     return false; // Always disarmed in Phase 1
   }

   bool safety_is_link_ok() {
     return false; // No input yet, so link is not ok
   }

   SystemError safety_get_error() {
     return current_error;
   }

   void safety_set_error(SystemError err) {
     if (current_error == ERR_NONE) {  // Don't overwrite first error
       current_error = err;
     }
   }
   ```

4. **Update diagnostics** to blink error codes:
   ```cpp
   // In diagnostics.cpp, add error code blinking
   // ERR_NONE: Slow blink (1Hz)
   // ERR_LOOP_OVERRUN: Fast blink (5Hz)
   // ERR_WATCHDOG_RESET: Rapid blink (10Hz)
   ```

5. **Update main.cpp** to use watchdog:
   ```cpp
   void setup() {
     // CRITICAL: Initialize outputs FIRST
     actuators_init();

     // Then initialize other modules
     safety_init();        // Enables watchdog
     diagnostics_init();
     state_init();
   }

   void loop() {
     static uint32_t next_loop_ms = 0;
     uint32_t now = millis();

     if (now >= next_loop_ms) {
       next_loop_ms = now + LOOP_PERIOD_MS;

       // Measure loop time
       uint32_t loop_start = micros();

       control_loop_tick();

       uint32_t loop_duration = micros() - loop_start;
       if (loop_duration > 10000) {  // >10ms
         safety_set_error(ERR_LOOP_OVERRUN);
       }

       // Pet watchdog LAST (after loop completes)
       safety_pet_watchdog();
     }
   }
   ```

#### Dependencies
- Requires: Phase 1.5 complete
- Enables: Automatic recovery from system lockups

#### Testing Strategy
- **Watchdog test**: Add infinite loop in code, verify system resets within ~1 second
- **Error code test**: Trigger each error, verify LED pattern changes
- **Normal operation**: Verify watchdog doesn't trigger during normal loops
- **Memory test**: Check build output shows SRAM usage <512 bytes

#### Acceptance Criteria
- [ ] Watchdog enabled and functional (test with intentional lockup)
- [ ] System recovers from watchdog reset with error code set
- [ ] LED blinks error code pattern
- [ ] Build output shows memory usage
- [ ] Normal operation doesn't trigger watchdog (100+ iterations)

#### Risks & Mitigations
| Risk | Impact | Mitigation |
|------|--------|------------|
| Watchdog triggers falsely during init | High | Pet watchdog in setup() if init takes >1s |
| Loop overrun false positives | Medium | Measure without debug Serial prints |
| Error codes not visible | Low | Document LED patterns clearly |

---

## Design Principles Applied

### SOLID Application
- **SRP**: Each module has single responsibility (config=constants, actuators=output, diagnostics=status, etc.)
- **OCP**: Modules extensible via adding functions; core initialization closed to modification
- **LSP**: Not applicable (no inheritance in Phase 1)
- **ISP**: Each module exposes minimal public API; internal details hidden
- **DIP**: Modules depend on RuntimeState abstraction, not concrete implementations

### DRY Considerations
- Pin definitions centralized in config.h
- Safe state logic in single function `actuators_set_safe_state()`
- Status LED patterns centralized in diagnostics module

### Complexity Management
- Each module <200 lines of code
- No function >50 lines
- No nesting >3 levels
- Clear naming conventions (module_function pattern)

## Testing Strategy

### Unit Testing
- **config module**: Verify constants defined correctly
- **actuators module**: Verify safe state sets all pins correctly
- **diagnostics module**: Verify LED blink timing
- **safety module**: Verify API returns safe defaults

### Integration Testing
- **Boot sequence**: Verify initialization order (outputs first)
- **Control loop**: Verify 100 Hz timing maintained
- **State consistency**: Verify RuntimeState accessed correctly

### Manual Testing
- **Power cycle test**: 20+ cycles, all outputs safe
- **Scope verification**: No transient spikes on motor pins
- **ESC verification**: Weapon ESC receives continuous min throttle
- **LED verification**: Visible blink pattern
- **Memory verification**: SRAM usage from build output

## Risk Assessment

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Weapon ESC misinterprets init signal | Low | Critical | Use Servo library; verify with oscilloscope; test 20+ boot cycles |
| Timer conflicts (Servo + PWM) | Medium | High | Review Arduino timer allocation; test early |
| SRAM usage exceeds budget | Low | Medium | Monitor build output; use static allocation only |
| L293D shield pin conflicts | Medium | Medium | Document pin usage; verify against shield schematic |
| LED not visible | Low | Low | Choose bright LED; test visibility before integration |
| Loop timing unstable | Low | Medium | Measure with micros(); avoid blocking operations |

## Open Questions

- [x] L293D shield pin assignments (standard shield pinout assumed)
- [x] Status LED location (using built-in LED)
- [ ] Serial debug enabled in Phase 1? (Recommend: disabled to save resources)
- [ ] ESC calibration needed before Phase 1 complete? (Defer to Phase 5)

---
**Status**: ready
**Created**: 2025-12-25
**Author**: feature-architect (via /new-project)
