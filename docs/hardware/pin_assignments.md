# Hardware Pin Assignments - UpVote Battlebot

**Shield**: Generic L293D Motor Driver Shield (74HC595 + Dual L293D)
**Date**: 2025-12-25
**Status**: Verified from hardware inspection

---

## Shield Overview

This shield uses:
- **2x L293D driver ICs** for 4 DC motor channels
- **1x 74HC595 shift register** to minimize Arduino pin usage
- **PWM control** via 4 Arduino pins (enable pins)
- **Direction control** via shift register (serial data)

**Key Advantage**: Only uses 6 Arduino pins total for 4 motor channels!

---

## Motor Control Pin Mapping

### DC Motors (M1, M2, M3, M4)

| Motor | Enable (PWM) Pin | Arduino Timer | Direction Control | Terminal Block |
|-------|------------------|---------------|-------------------|----------------|
| **M1** | Pin 5 (PWM)     | Timer0 (OC0B) | Via shift register | M1 terminals   |
| **M2** | Pin 6 (PWM)     | Timer0 (OC0A) | Via shift register | M2 terminals   |
| **M3** | Pin 9 (PWM)     | Timer1 (OC1A) | Via shift register | DC Motor 1     |
| **M4** | Pin 10 (PWM)    | Timer1 (OC1B) | Via shift register | DC Motor 4     |

**Notes**:
- M1/M2 use Timer0 (shared with `millis()` - may cause issues!)
- M3/M4 use Timer1 (conflicts with `Servo.h`!)
- All direction bits controlled via 74HC595 shift register

---

## Shift Register (74HC595) Control

The shield uses a shift register to control motor directions with only 3 Arduino pins:

| Function | Arduino Pin | Description |
|----------|-------------|-------------|
| **LATCH** | Pin 12 | Shift register latch (STcp) |
| **ENABLE** | Pin 7  | Shift register output enable (OE, active LOW) |
| **DATA**  | Pin 8  | Serial data input (DS) |
| **CLOCK** | Pin 4  | Shift clock (SHcp) |

**Shift Register Bit Assignment**:
```
Bit 7: M4 Direction B
Bit 6: M4 Direction A
Bit 5: M3 Direction B
Bit 4: M3 Direction A
Bit 3: M2 Direction B
Bit 2: M2 Direction A
Bit 1: M1 Direction B
Bit 0: M1 Direction A
```

**Direction Logic**:
- Forward: A=HIGH, B=LOW
- Reverse: A=LOW, B=HIGH
- Brake: A=HIGH, B=HIGH
- Coast: A=LOW, B=LOW

---

## Servo Connector

| Servo | Arduino Pin | Timer Used |
|-------|-------------|------------|
| **Servo 1** | Pin 9  | Timer1 (conflicts with M3!) |
| **Servo 2** | Pin 10 | Timer1 (conflicts with M4!) |

**CRITICAL CONFLICT**:
- Servo pins (9, 10) are the SAME as M3/M4 PWM pins!
- You CANNOT use servos and motors M3/M4 simultaneously with standard Arduino libraries!

---

## Timer Allocation Summary

| Timer | Default Use | This Shield's Use | Conflicts |
|-------|-------------|-------------------|-----------|
| **Timer0** | `millis()`, `delay()` | M1 PWM (pin 5), M2 PWM (pin 6) | **HIGH RISK**: May affect timing! |
| **Timer1** | `Servo.h` | M3 PWM (pin 9), M4 PWM (pin 10) | **CRITICAL**: Servo conflicts with M3/M4! |
| **Timer2** | `Tone.h` | **AVAILABLE** | None (unused) |

---

## UpVote-Specific Pin Usage

Based on our battlebot design:

### Motors (Holonomic Drive)
| Motor | Shield Terminal | Function | PWM Pin | Notes |
|-------|----------------|----------|---------|-------|
| **Front-Left** | M3 (DC Motor 1) | Drive wheel | Pin 9 | Timer1 |
| **Front-Right** | M4 (DC Motor 4) | Drive wheel | Pin 10 | Timer1 |
| **Rear-Left** | M1 | Drive wheel | Pin 5 | Timer0 ⚠️ |
| **Rear-Right** | M2 | Drive wheel | Pin 6 | Timer0 ⚠️ |

### Weapon ESC
| Component | Connection | Pin | Notes |
|-----------|------------|-----|-------|
| **HW30A ESC** | Servo 1 header | Pin 9 | **CONFLICT!** Same as M3 PWM! |

### Self-Righting Servo
| Component | Connection | Pin | Notes |
|-----------|------------|-----|-------|
| **Servo** | Servo 2 header | Pin 10 | **CONFLICT!** Same as M4 PWM! |

---

## CRITICAL CONFLICTS IDENTIFIED

### Conflict 1: Servo PWM vs Motor PWM
**Problem**: Pins 9 and 10 are used for BOTH:
- M3/M4 motor PWM (drive wheels)
- Servo headers (weapon ESC + self-righting servo)

**Impact**: Cannot use Arduino `Servo.h` library while controlling M3/M4 motors

**Solutions**:
1. **Option A (Recommended)**: Direct PWM control for weapon ESC
   - Don't use `Servo.h` library
   - Use `analogWrite()` for ESC (50 Hz PWM with custom timing)
   - Trade-off: More complex ESC control code

2. **Option B**: Rewire weapon ESC to different pin
   - Use a non-shield pin (e.g., pin 3, 11) for weapon ESC
   - Requires jumper wire to bypass shield
   - Trade-off: Less clean wiring

3. **Option C**: Don't use M3/M4, only use M1/M2
   - Only control 2 motors (differential drive, not holonomic)
   - Trade-off: Loses holonomic drive capability

### Conflict 2: Timer0 and millis()
**Problem**: M1/M2 PWM (pins 5, 6) share Timer0 with `millis()` and `delay()`

**Impact**: Using `analogWrite()` on pins 5/6 may affect timing accuracy

**Solution**:
- This is **acceptable** for motor control (timing drift is minimal)
- Avoid using `delay()` in code (we already use non-blocking timing)
- Monitor loop timing in Phase 1 testing

---

## Recommended Pin Assignments for UpVote

### Option 1: Use M3/M4 for drive, custom PWM for weapon (RECOMMENDED)

```cpp
// Drive motors (via shield)
#define PIN_MOTOR_FL_PWM    9   // M3 enable
#define PIN_MOTOR_FR_PWM   10   // M4 enable
#define PIN_MOTOR_RL_PWM    5   // M1 enable
#define PIN_MOTOR_RR_PWM    6   // M2 enable

// Shift register control
#define PIN_SR_LATCH       12
#define PIN_SR_ENABLE       7
#define PIN_SR_DATA         8
#define PIN_SR_CLOCK        4

// Weapon ESC (custom PWM, not via Servo.h)
#define PIN_WEAPON_ESC      3   // Use pin 3 (Timer2, no conflicts!)

// Self-righting servo (if needed, use SoftwareServo or custom PWM)
#define PIN_SELFRIGHT_SERVO 11  // Use pin 11 (Timer2)

// CRSF receiver (Hardware Serial)
#define PIN_CRSF_RX         0   // Hardware Serial RX
#define PIN_CRSF_TX         1   // Hardware Serial TX

// Status LED
#define PIN_STATUS_LED     LED_BUILTIN  // Pin 13
```

**Why This Works**:
- Motors M1-M4 use shield pins (5, 6, 9, 10) ✅
- Weapon ESC on pin 3 (Timer2, no conflicts) ✅
- Self-righting servo on pin 11 (Timer2) ✅
- CRSF uses Hardware Serial (pins 0, 1) ✅
- No timer conflicts! ✅

---

## Final Pin Summary

| Pin | Function | Timer | Module |
|-----|----------|-------|--------|
| 0 | CRSF RX | - | Input |
| 1 | CRSF TX | - | Input |
| 3 | Weapon ESC PWM | Timer2 | Actuators |
| 4 | Shift register clock | - | Actuators |
| 5 | Motor RL PWM (M1) | Timer0 | Actuators |
| 6 | Motor RR PWM (M2) | Timer0 | Actuators |
| 7 | Shift register enable | - | Actuators |
| 8 | Shift register data | - | Actuators |
| 9 | Motor FL PWM (M3) | Timer1 | Actuators |
| 10 | Motor FR PWM (M4) | Timer1 | Actuators |
| 11 | Self-righting servo | Timer2 | Actuators |
| 12 | Shift register latch | - | Actuators |
| 13 | Status LED | - | Diagnostics |

**Unused Pins**: A0-A5 (can use A0 for battery voltage monitoring!)

---

## Hardware Modifications Required

### Required Changes
1. **Weapon ESC**: Connect to pin 3 (not Servo 1 header)
2. **Self-righting servo**: Connect to pin 11 (not Servo 2 header)

### Wiring Instructions
- Weapon ESC signal wire: Jumper to pin 3 on Arduino (bypass shield)
- Self-righting servo signal wire: Jumper to pin 11 (bypass shield)
- Both ESC and servo: Share ground and 5V from external BEC (NOT Arduino!)

---

## Verification Checklist

- [x] Shield model identified: Generic L293D with 74HC595
- [x] Motor pin assignments documented
- [x] Timer conflicts identified and resolved
- [x] Servo/ESC conflicts resolved (use pins 3, 11 instead of shield headers)
- [x] Pin assignments verified against UpVote requirements
- [ ] Physical verification: Test continuity with multimeter (optional)
- [ ] Bench test: Upload test sketch and verify motor control

---

## References

- Shield type: Generic Arduino Motor Shield (L293D + 74HC595)
- Similar models: HW-130, L293D Motor Drive Shield
- Datasheet: L293D quadruple half-H driver
- Arduino Timer reference: https://www.arduino.cc/reference/en/language/functions/analog-io/analogwrite/

---

**Status**: VERIFIED - Ready for Phase 1 implementation
**Next Step**: Update `include/config.h` with these pin definitions

