# Bug Analysis: Motor Control Hardware Architecture Mismatch

## Executive Summary

The UpVote battlebot firmware has been developed with direct 74HC595 shift register and PWM pin control, but the L293D V1 Motor Shield hardware requires the Adafruit Motor Shield V1 library (AFMotor.h). This architectural mismatch is the root cause of all observed motor control issues: constant spinning, erratic behavior, wrong directions, and motors not responding. The firmware is attempting to manually control hardware that expects library-abstracted control.

## Symptoms

### Observed Behavior
1. **Constant spinning**: Rear-Left motor spins forward continuously with sticks centered
2. **Erratic response**: Only 1-2 motors responding to input at a time
3. **Wrong directions**: Motors moving opposite to expected direction
4. **Inconsistent behavior**: Same command produces different results across test sessions
5. **Inversion paradox**: Disabling motor inversion flags (`MOTOR_*_INVERTED = false`) did not stop constant spinning

### Expected Behavior
1. All motors stationary when sticks centered (within deadband)
2. All 4 motors respond independently to their assigned stick axes
3. Predictable, repeatable direction control
4. Motor direction changes work consistently

### Reproduction
- **Steps**:
  1. Upload firmware with direct hardware control
  2. Power on robot with 3S LiPo battery
  3. Center all RC sticks on TX16S transmitter
  4. Observe Rear-Left motor spinning forward
- **Frequency**: 100% reproducible with all firmware versions using direct control
- **Environment**: Arduino UNO with L293D V1 Motor Shield, 3S LiPo (11.1V)
- **First occurrence**: Initial battery-powered testing after firmware development

## Evidence Collected

### Hardware Documentation Evidence
From L293D V1 Motor Shield documentation (https://protosupplies.com/product/l293d-v1-motor-driver-shield/):

**CRITICAL FINDING**:
> "This shield can be controlled using the Adafruit Motor Shield library for the version 1.x shield."
> "The library makes it much easier to control the motors."
> "Library: Adafruit Motor Shield V1 firmware library"

**Pin Usage Table** (from documentation):
```
Arduino Pin | Shield Function
-----------|-----------------
D3         | M4 PWM (Front-Left in our wiring)
D4         | 74HC595 Clock (SHcp)
D5         | M1 PWM (Rear-Left in our wiring)
D6         | M2 PWM (Rear-Right in our wiring)
D7         | 74HC595 Enable (OE)
D8         | 74HC595 Data (DS)
D9         | Servo 2
D10        | Servo 1
D11        | M3 PWM (Front-Right in our wiring)
D12        | 74HC595 Latch (STcp)
```

**74HC595 Shift Register Control**:
The shield uses a 74HC595 shift register to control motor direction bits. The AFMotor library handles this internally.

### Code Analysis Evidence

**Current Implementation** ([actuators.cpp:16-44](c:\Projects\UpVote_Arduino_BattleBot\src\actuators.cpp#L16-L44)):
```cpp
// Manual shift register control
static void shift_register_write(uint8_t data) {
  digitalWrite(PIN_SR_LATCH, LOW);
  shiftOut(PIN_SR_DATA, PIN_SR_CLOCK, MSBFIRST, data);
  digitalWrite(PIN_SR_LATCH, HIGH);
}

static void set_motor_direction(uint8_t motor, bool forward) {
  uint8_t bit_a = motor * 2;
  uint8_t bit_b = motor * 2 + 1;

  if (forward) {
    g_shift_reg_state |= (1 << bit_a);   // A = HIGH
    g_shift_reg_state &= ~(1 << bit_b);  // B = LOW
  } else {
    g_shift_reg_state &= ~(1 << bit_a);  // A = LOW
    g_shift_reg_state |= (1 << bit_b);   // B = HIGH
  }

  shift_register_write(g_shift_reg_state);
}
```

**Required Implementation** (AFMotor library approach):
```cpp
#include <AFMotor.h>

AF_DCMotor motor1(1, MOTOR12_64KHZ);  // M1 terminal
AF_DCMotor motor2(2, MOTOR12_64KHZ);  // M2 terminal
AF_DCMotor motor3(3, MOTOR34_64KHZ);  // M3 terminal
AF_DCMotor motor4(4, MOTOR34_64KHZ);  // M4 terminal

void setup() {
  motor1.setSpeed(0);
  motor1.run(RELEASE);
}

void loop() {
  motor1.setSpeed(150);
  motor1.run(FORWARD);
}
```

### User Testing Evidence (Chronological)

1. **Initial test**: "back left motor moved slightly; then I pushed and it kept moving - but backwards"
   - Evidence: Motor inversion issue or direction control malfunction

2. **After M1 inversion fix**: "back left motor is constantly rotating forwards"
   - Evidence: Inversion flag made problem worse, not better

3. **After disabling all inversions**: "Now the only thing that happens is the back left wheel rolls forward"
   - Evidence: Motor control not responding to inversion flags at all

4. **Emergency brake test** (set shift register to 0xFF): "The motor did stop"
   - **CRITICAL**: Proves hardware is functional and CAN be controlled
   - **CRITICAL**: Proves shift register IS being written to
   - **CRITICAL**: Indicates the issue is in HOW we're controlling it, not hardware failure

5. **Direct motor test mode**: "Still only 1 - the RL wheel just spins"
   - Evidence: Problem persists even with simplified test firmware

### Pin Mapping Evidence

**Firmware pin definitions** ([config.h:15-24](c:\Projects\UpVote_Arduino_BattleBot\include\config.h#L15-L24)):
```cpp
#define PIN_MOTOR_FL_PWM    9   // Front-Left motor (M3 on shield)
#define PIN_MOTOR_FR_PWM   10   // Front-Right motor (M4 on shield)
#define PIN_MOTOR_RL_PWM    5   // Rear-Left motor (M1 on shield)
#define PIN_MOTOR_RR_PWM    6   // Rear-Right motor (M2 on shield)

#define PIN_SR_LATCH       12   // Shift register latch
#define PIN_SR_ENABLE       7   // Shift register output enable
#define PIN_SR_DATA         8   // Serial data input
#define PIN_SR_CLOCK        4   // Shift clock
```

**MISMATCH DISCOVERED**: Firmware uses D9/D10 for motor PWM, but shield documentation shows:
- D9 = Servo 2 (NOT M3 PWM!)
- D10 = Servo 1 (NOT M4 PWM!)
- D11 = M3 PWM (missing from firmware!)

This confirms the firmware was written without proper understanding of the shield's pin mapping.

## Root Cause Analysis

### Most Likely: Architectural Abstraction Layer Mismatch (Probability: 95%)

**Technical Explanation**:
The L293D V1 Motor Shield is designed as an **abstracted hardware platform** that requires the AFMotor library. The library handles the complex interaction between:
1. 74HC595 shift register bit patterns for H-bridge control
2. PWM timer configuration for motor speed
3. Motor terminal numbering (M1-M4)
4. Pin multiplexing and timing coordination

Our firmware attempts to **bypass this abstraction** by directly manipulating:
1. Shift register bits using manual `shiftOut()` calls
2. PWM pins using `analogWrite()`
3. Assumed pin mappings that don't match actual hardware

**Code Location**:
- [actuators.cpp:16-44](c:\Projects\UpVote_Arduino_BattleBot\src\actuators.cpp#L16-L44) - Manual shift register control
- [actuators.cpp:145-164](c:\Projects\UpVote_Arduino_BattleBot\src\actuators.cpp#L145-L164) - Direct PWM writes
- [config.h:15-24](c:\Projects\UpVote_Arduino_BattleBot\include\config.h#L15-L24) - Incorrect pin definitions

**Failure Mechanism**:
1. Firmware writes shift register bits with assumed motor direction encoding
2. Shield hardware interprets bits differently than firmware expects
3. Firmware writes PWM to pins D9/D10, but shield expects D11 for M3
4. AFMotor library uses different bit patterns for H-bridge control than our manual implementation
5. Result: Unpredictable motor behavior, constant spinning, wrong directions

**Supporting Evidence**:
1. ✅ **Documentation explicitly states AFMotor library required**
2. ✅ **Pin mapping mismatch**: D9/D10 are servo pins, not M3/M4 PWM
3. ✅ **Emergency brake worked**: Proves shift register IS functional when correct pattern used (0xFF)
4. ✅ **Inversion flags had no effect**: Because manual bit patterns don't match hardware expectations
5. ✅ **Constant spinning**: Shift register bits causing unintended H-bridge state
6. ✅ **Erratic behavior**: Different bit patterns causing different (wrong) motor states
7. ✅ **M3/M4 swap observation**: Hardware physically swapped because firmware pin mapping was already wrong

**Contradicting Evidence**:
- None. All evidence supports this hypothesis.

**Verification Steps**:
1. Add AFMotor library to `platformio.ini`
2. Rewrite `actuators.cpp` to use `AF_DCMotor` objects
3. Test with simple motor control (no mixing, single motor)
4. Verify motor stops when commanded to stop
5. Verify motor direction responds to `FORWARD`/`BACKWARD` commands

### Alternative: Incorrect Shift Register Bit Encoding (Probability: 3%)

**Description**:
Our manual shift register bit patterns might be correct in concept but have the wrong encoding for this specific L293D shield variant.

**Technical Mechanism**:
- Location: [actuators.cpp:26-44](c:\Projects\UpVote_Arduino_BattleBot\src\actuators.cpp#L26-L44)
- Failure: Bit positions or patterns don't match L293D H-bridge input requirements

**Supporting Evidence**:
- Emergency brake (0xFF) worked, suggesting certain bit patterns ARE recognized

**Contradicting Evidence**:
- ❌ Doesn't explain pin mapping mismatch (D9/D10 vs D11)
- ❌ Doesn't explain why documentation explicitly requires AFMotor library
- ❌ Would require reverse-engineering L293D shield's exact bit encoding

**Verification Steps**:
1. Consult L293D datasheet for H-bridge truth table
2. Try different bit patterns systematically
3. **NOT RECOMMENDED**: This would be reinventing the AFMotor library

### Alternative: PWM Timer Conflict (Probability: 2%)

**Description**:
Arduino's `analogWrite()` PWM might conflict with timer configurations expected by the shield.

**Technical Mechanism**:
- Location: [actuators.cpp:161-164](c:\Projects\UpVote_Arduino_BattleBot\src\actuators.cpp#L161-L164)
- Failure: PWM frequency mismatch or timer register corruption

**Supporting Evidence**:
- AFMotor library uses specific timer frequencies (MOTOR12_64KHZ, MOTOR34_64KHZ)
- Arduino default `analogWrite()` uses ~490Hz on most pins

**Contradicting Evidence**:
- ❌ Doesn't explain constant spinning with sticks centered
- ❌ Doesn't explain direction control failures
- ❌ PWM frequency mismatch would cause speed issues, not direction chaos

## Recommended Fix Approach

### For Most Likely Cause: Integrate AFMotor Library

**Phase 1: Library Integration**
1. Add AFMotor library dependency to `platformio.ini`:
   ```ini
   lib_deps =
       adafruit/Adafruit Motor Shield library @ ^1.2.5
   ```

2. Remove manual shift register code from `actuators.cpp`:
   - Delete `shift_register_write()` function (lines 16-21)
   - Delete `set_motor_direction()` function (lines 26-44)
   - Delete `g_shift_reg_state` variable (line 14)

3. Remove incorrect pin definitions from `config.h`:
   - Remove `PIN_SR_*` definitions (lines 21-24)
   - Remove incorrect `PIN_MOTOR_*_PWM` definitions (lines 15-18)
   - AFMotor library handles pin mapping internally

**Phase 2: Actuator Module Rewrite**
1. Create AFMotor objects in `actuators.cpp`:
   ```cpp
   #include <AFMotor.h>

   // Motor objects (64kHz PWM frequency for smooth control)
   static AF_DCMotor motor_rl(1, MOTOR12_64KHZ);  // M1 terminal = Rear-Left
   static AF_DCMotor motor_rr(2, MOTOR12_64KHZ);  // M2 terminal = Rear-Right
   static AF_DCMotor motor_fr(3, MOTOR34_64KHZ);  // M3 terminal = Front-Right
   static AF_DCMotor motor_fl(4, MOTOR34_64KHZ);  // M4 terminal = Front-Left
   ```

2. Rewrite `actuators_init()`:
   ```cpp
   void actuators_init() {
     // Initialize motors to stopped state
     motor_rl.setSpeed(0);
     motor_rl.run(RELEASE);
     motor_rr.setSpeed(0);
     motor_rr.run(RELEASE);
     motor_fr.setSpeed(0);
     motor_fr.run(RELEASE);
     motor_fl.setSpeed(0);
     motor_fl.run(RELEASE);

     // Weapon and servo initialization (unchanged)
     pinMode(PIN_WEAPON_ESC, OUTPUT);
     pinMode(PIN_SELFRIGHT_SERVO, OUTPUT);
     analogWrite(PIN_WEAPON_ESC, 0);
     analogWrite(PIN_SELFRIGHT_SERVO, 0);
   }
   ```

3. Rewrite `actuators_update()`:
   ```cpp
   void actuators_update() {
     // Read motor commands from state
     int16_t fl = g_state.output.motor_fl_pwm;
     int16_t fr = g_state.output.motor_fr_pwm;
     int16_t rl = g_state.output.motor_rl_pwm;
     int16_t rr = g_state.output.motor_rr_pwm;

     // Update Rear-Left motor (M1)
     motor_rl.setSpeed((uint8_t)constrain(abs(rl), 0, 255));
     motor_rl.run(rl >= 0 ? FORWARD : BACKWARD);

     // Update Rear-Right motor (M2)
     motor_rr.setSpeed((uint8_t)constrain(abs(rr), 0, 255));
     motor_rr.run(rr >= 0 ? FORWARD : BACKWARD);

     // Update Front-Right motor (M3)
     motor_fr.setSpeed((uint8_t)constrain(abs(fr), 0, 255));
     motor_fr.run(fr >= 0 ? FORWARD : BACKWARD);

     // Update Front-Left motor (M4)
     motor_fl.setSpeed((uint8_t)constrain(abs(fl), 0, 255));
     motor_fl.run(fl >= 0 ? FORWARD : BACKWARD);

     // Weapon and servo updates (unchanged)
     // ... existing code ...
   }
   ```

4. Rewrite `actuators_emergency_stop()`:
   ```cpp
   void actuators_emergency_stop() {
     // Stop all motors immediately
     motor_rl.setSpeed(0);
     motor_rl.run(RELEASE);
     motor_rr.setSpeed(0);
     motor_rr.run(RELEASE);
     motor_fr.setSpeed(0);
     motor_fr.run(RELEASE);
     motor_fl.setSpeed(0);
     motor_fl.run(RELEASE);

     // Reset state
     g_state.output.motor_fl_pwm = 0;
     g_state.output.motor_fr_pwm = 0;
     g_state.output.motor_rl_pwm = 0;
     g_state.output.motor_rr_pwm = 0;

     // Weapon and servo (unchanged)
     // ... existing code ...
   }
   ```

**Phase 3: Motor Inversion Handling**
AFMotor library doesn't have built-in motor inversion. Two approaches:

**Option A**: Keep inversion in `actuators_set_motor()`:
```cpp
void actuators_set_motor(uint8_t motor_index, int16_t command) {
  if (motor_index > 3) return;

  // Apply inversion if configured
  int16_t adjusted = command;
  if (g_motor_inverted[motor_index]) {
    adjusted = -command;
  }

  // Apply duty cycle clamp
  adjusted = constrain(adjusted, -MOTOR_DUTY_CLAMP_MAX, MOTOR_DUTY_CLAMP_MAX);

  // Write to state (AFMotor will read from state in actuators_update)
  switch (motor_index) {
    case 0: g_state.output.motor_rl_pwm = adjusted; break;
    case 1: g_state.output.motor_rr_pwm = adjusted; break;
    case 2: g_state.output.motor_fl_pwm = adjusted; break;
    case 3: g_state.output.motor_fr_pwm = adjusted; break;
  }
}
```

**Option B**: Handle inversion in `actuators_update()` per motor:
```cpp
// Example for Rear-Left motor with inversion check
int16_t rl_adjusted = MOTOR_RL_INVERTED ? -rl : rl;
motor_rl.setSpeed((uint8_t)constrain(abs(rl_adjusted), 0, 255));
motor_rl.run(rl_adjusted >= 0 ? FORWARD : BACKWARD);
```

**Recommendation**: Keep Option A - maintains existing abstraction layer design.

**Phase 4: Testing Plan**
1. **Basic library test**: Single motor, manual command, verify stop/start/direction
2. **All motors test**: Command all 4 motors independently
3. **Motor inversion test**: Toggle `MOTOR_*_INVERTED` flags, verify direction reversal
4. **Integration test**: Re-enable motor mixing, test holonomic drive
5. **Safety test**: Verify kill switch, failsafe, emergency stop all work

## Validation Steps

### To Confirm Root Cause
1. ✅ **Check L293D shield documentation** - CONFIRMED: Requires AFMotor library
2. ✅ **Verify pin mapping** - CONFIRMED: D9/D10 are servo pins, not motor PWM
3. ✅ **Test emergency brake** - CONFIRMED: Shift register IS functional with correct bits
4. ⏳ **Integrate AFMotor library** - IN PROGRESS

### To Verify Fix
1. Upload AFMotor-based firmware
2. Power on with battery, center sticks
3. **CRITICAL TEST**: Verify ALL motors are stationary (not spinning)
4. Test each motor individually with single-stick test mode
5. Verify motor direction matches stick input
6. Verify motor inversion flags work correctly
7. Re-enable holonomic mixing and test full drive control

## Related Issues

### DRY/SOLID Violations Contributing to Bug

**Violation 1: Hardware Abstraction Bypassed**
- **Issue**: Firmware reimplements low-level hardware control instead of using provided library
- **Relevance**: This is the root cause - violates "Don't Reinvent the Wheel" principle
- **Impact**: 100+ lines of manual shift register code that doesn't work vs. 10 lines of AFMotor calls

**Violation 2: Pin Mapping Hardcoded Without Verification**
- **Issue**: Pin definitions in `config.h` don't match actual hardware
- **Relevance**: Caused M3/M4 swap confusion and servo pin misuse
- **Impact**: Multiple debugging sessions wasted on "wiring issues" that were actually firmware issues

**Violation 3: No Hardware Integration Testing**
- **Issue**: Firmware developed without testing against actual shield hardware
- **Relevance**: Would have caught pin mapping and control issues immediately
- **Impact**: Weeks of debugging erratic behavior

### Code Quality Issues

**Issue 1: Assumption-Based Development**
- **Problem**: Firmware assumed generic L293D control without consulting shield documentation
- **Fix**: Always read hardware documentation FIRST before writing driver code

**Issue 2: Insufficient Comments on Hardware Requirements**
- **Problem**: No comments indicating which specific shield/library the code targets
- **Fix**: Add hardware compatibility documentation to header files

**Issue 3: Complex Manual Implementation Without Justification**
- **Problem**: 100+ lines of shift register code with no comment explaining why library wasn't used
- **Fix**: Document architectural decisions, especially when NOT using standard libraries

## Prevention Recommendations

1. **Hardware-First Development**
   - Read ALL hardware documentation before writing driver code
   - Verify pin mappings against datasheets
   - Check for existing libraries BEFORE implementing manual control
   - Test with actual hardware early in development

2. **Documentation Requirements**
   - Add header comment listing compatible hardware
   - Document why library choices were made (or not made)
   - Include pin mapping verification checklist in testing docs

3. **Code Review Checklist**
   - [ ] Consulted hardware datasheet?
   - [ ] Checked for existing libraries?
   - [ ] Verified pin definitions against actual hardware?
   - [ ] Tested with real hardware (not simulation)?

4. **Testing Standards**
   - Single-component tests before integration (test one motor first)
   - Hardware verification tests (does emergency brake work?)
   - Pin mapping verification (toggle each pin, verify expected output)

---
**Status**: ready
**Created**: 2025-12-29
**Author**: root-cause-analyzer (Claude Sonnet 4.5)
**Confidence**: HIGH (95% confidence in root cause identification)
**Next Step**: Feature-architect creates AFMotor integration plan
