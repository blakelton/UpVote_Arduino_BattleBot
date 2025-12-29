# UpVote Battlebot - Troubleshooting Guide

**Version**: 1.0
**Firmware**: Phases 1-6 Complete
**Last Updated**: 2025-12-26

---

## Table of Contents

1. [Quick Diagnostic Flowchart](#quick-diagnostic-flowchart)
2. [LED Pattern Diagnostics](#led-pattern-diagnostics)
3. [Common Issues](#common-issues)
4. [Power-On Problems](#power-on-problems)
5. [Control Issues](#control-issues)
6. [Weapon Arming Problems](#weapon-arming-problems)
7. [Link/Telemetry Issues](#linktelemetry-issues)
8. [Advanced Diagnostics](#advanced-diagnostics)
9. [Hardware Fault Isolation](#hardware-fault-isolation)

---

## Quick Diagnostic Flowchart

```
Bot won't power on?
├─ Battery connected? → NO → Connect battery
├─ Battery charged? → NO → Charge battery (12.6V for 3S)
├─ Power switch on? → NO → Turn on power switch
└─ LED lights up? → NO → Check Arduino power LED
                  → YES → See LED pattern below

LED pattern abnormal?
├─ Solid ON → FAILSAFE active → Check [LED Pattern Diagnostics](#led-pattern-diagnostics)
├─ Blinking N times → ERROR code N → See [Error Codes](#error-codes)
└─ Slow blink (1 Hz) → NORMAL, check controls

Controls not responding?
├─ Transmitter on? → NO → Turn on TX16S
├─ Correct model? → NO → Select UpVote model
├─ Link shown? → NO → See [Link/Telemetry Issues](#linktelemetry-issues)
└─ Sticks centered? → NO → Center sticks

Weapon won't arm?
├─ SA switch UP? → NO → Move SA to UP position
├─ Slider at 0%? → NO → Move slider to bottom
├─ Kill switch DOWN? → NO → Move SD to DOWN
├─ Link OK? → NO → Fix link first
└─ LED fast blink? → NO → See [Weapon Arming Problems](#weapon-arming-problems)
```

---

## LED Pattern Diagnostics

### LED Behavior Reference

| Pattern | Meaning | Status | Next Step |
|---------|---------|--------|-----------|
| **Off** | No power | 🔴 | Check battery connection, power switch |
| **Slow Blink (1 Hz)** | SAFE - Normal operation | ✅ | Ready to operate |
| **Fast Blink (5 Hz)** | ARMED - Weapon live | ⚠️ | Weapon is active, exercise caution |
| **Solid ON** | FAILSAFE - Emergency mode | 🔴 | Check transmitter, kill switch |
| **1 blink + pause** | Loop overrun error | 🔴 | Reflash firmware if persistent |
| **2 blinks + pause** | Watchdog reset | 🔴 | Power cycle, check for shorts |
| **3 blinks + pause** | CRSF timeout (link loss) | 🔴 | Check receiver, antenna, transmitter |
| **4 blinks + pause** | CRSF CRC error | 🔴 | Check receiver wiring, interference |

### Error Code Details

#### Error Code 1: Loop Overrun
**Symptom**: LED blinks once, pauses ~2 seconds, repeats

**Meaning**: Control loop took longer than 10ms (exceeded 100 Hz deadline)

**Likely Causes**:
- Firmware bug (shouldn't happen in normal operation)
- Electrical noise causing CPU lockup
- Faulty flash memory

**Immediate Action**:
1. Complete current match safely (disarm weapon)
2. Power cycle bot
3. If persists, reflash firmware

**Long-Term Fix**:
- Check for electrical noise sources
- Verify power supply stability
- Re-upload firmware from GitHub

---

#### Error Code 2: Watchdog Reset
**Symptom**: LED blinks twice, pauses ~2 seconds, repeats

**Meaning**: System recovered from a freeze/crash (watchdog timer triggered)

**Likely Causes**:
- Power brownout (voltage drop)
- Electrical short
- EMI interference from motors/weapon
- Firmware infinite loop (very unlikely)

**Immediate Action**:
1. **Power cycle** the bot
2. **Check battery voltage** (should be >7.0V under load)
3. **Inspect wiring** for shorts or damage

**Diagnostic Steps**:
1. Disconnect weapon motor → Test drive only
   - If error clears: Weapon circuit issue
   - If error persists: Power or CPU issue

2. Check voltage under load:
   - Connect multimeter to battery
   - Run motors at full power
   - Voltage should stay >6.8V
   - If drops <6.5V: Battery weak or wiring resistance

3. Inspect for shorts:
   - Visual inspection of all connections
   - Check motor shield for solder bridges
   - Verify no exposed wires touching

---

#### Error Code 3: CRSF Timeout
**Symptom**: LED blinks three times, pauses ~2 seconds, repeats

**Meaning**: No valid CRSF packets received for >200ms (link loss)

**Likely Causes**:
- Transmitter turned off
- Receiver not bound to transmitter
- Receiver antenna damaged
- Out of range
- RF interference

**Immediate Action**:
1. **Verify transmitter is ON** and correct model selected
2. **Check receiver LED** (CR8 Nano):
   - Should be **solid/blinking** if bound
   - If **off**: No power to receiver
   - If **rapid flash**: Not bound

**Diagnostic Steps**:

**Step 1: Power Check**
- Measure voltage at receiver (should be 5V)
- Check 5V BEC output
- Verify receiver power wires connected

**Step 2: Binding Check**
- Check TX16S model settings → ExpressLRS bind
- Receiver LED pattern:
  - **Slow blink**: Bound, waiting for TX
  - **Fast blink**: Bind mode active (not bound)
  - **Solid**: Connected to TX

**Step 3: Re-bind if needed**
1. On TX16S: Model Settings → ExpressLRS → Bind
2. Power on receiver while holding bind button (if available)
3. Wait for receiver LED to go solid
4. Exit bind mode on TX16S

**Step 4: Range Test**
- Start close to bot (<1 meter)
- Verify controls work
- Slowly increase distance
- If fails at short range: Antenna or receiver issue
- If fails at long range: Normal (ExpressLRS range varies)

---

#### Error Code 4: CRSF CRC Error
**Symptom**: LED blinks four times, pauses ~2 seconds, repeats

**Meaning**: Received CRSF packets failed CRC validation (corrupted data)

**Likely Causes**:
- Electrical noise on UART lines
- Loose receiver connection
- Damaged receiver
- EMI from motors/ESC/weapon

**Immediate Action**:
1. **Power cycle** bot and transmitter
2. **Check receiver wiring**:
   - TX → Arduino RX (pin 0)
   - RX → Arduino TX (pin 1)
   - Both wires should be **twisted pair** or **shielded**

**Diagnostic Steps**:

**Step 1: Wiring Inspection**
- Verify receiver TX → Arduino RX (pin 0)
- Verify receiver RX → Arduino TX (pin 1)
- Check for:
  - Loose connections
  - Broken wires
  - Oxidized pins
  - Wrong pin mapping

**Step 2: Noise Reduction**
- **Separate power rails**: Logic 5V away from motor power
- **Twist UART wires**: Reduces EMI pickup
- **Add capacitors**: 100µF on motor power lines
- **Ferrite beads**: On motor and ESC wires if available

**Step 3: Receiver Test**
- Swap receiver with known-good unit
- If error clears: Original receiver faulty
- If error persists: Noise issue or Arduino problem

**Step 4: EMI Test**
- Disconnect weapon motor
- Test with drive motors only
- If CRC errors stop: Weapon motor EMI issue
  - Add capacitor across weapon motor terminals
  - Improve weapon motor grounding

---

## Common Issues

### Issue: Bot Powers On But Won't Move

**Symptoms**:
- LED slow blink (normal)
- Transmitter shows link OK
- Sticks don't control movement

**Checklist**:

1. **Kill Switch Check**:
   - [ ] SD switch in DOWN position (inactive)
   - If SD is UP: LED will be solid ON (failsafe)

2. **Stick Calibration**:
   - [ ] Center both sticks
   - [ ] Try moving right stick forward gently
   - [ ] Check TX16S calibration if needed

3. **Link Verification**:
   - [ ] TX16S shows RSSI signal
   - [ ] Telemetry data updating (Batt voltage, RAM%)

4. **Motor Connections**:
   - [ ] All motor wires connected to L293D shield
   - [ ] Shield seated properly on Arduino
   - [ ] Shield power jumper configured correctly

**Advanced**:
- Check motor polarity flags in `config.h`
- Verify L293D shield enable pin (should be LOW)
- Test motors individually (requires test firmware)

---

### Issue: Weapon Won't Arm

**Symptoms**:
- Move SA switch UP
- LED stays slow blink (doesn't go to fast blink)
- Weapon doesn't spin

**Arming Preconditions** (ALL must be true):

1. [ ] **ARM switch (SA)** in UP position
2. [ ] **Kill switch (SD)** in DOWN position
3. [ ] **Weapon slider** at 0-3% (bottom position)
4. [ ] **Link healthy** (transmitter connected, no errors)
5. [ ] **No system errors** (LED not blinking error code)

**Diagnostic Steps**:

**Step 1: Verify Slider Position**
- Move weapon slider to **absolute bottom** (0%)
- Wait 1 second
- Move SA to UP
- LED should change to fast blink

**Step 2: Check Throttle Hysteresis**
- If you previously disarmed with slider >3%:
  - You MUST move slider below 10% first
  - Then you can move slider to 0% and re-arm
- **Solution**: Move slider all the way down, wait 2 seconds, try arming again

**Step 3: Verify Link**
- Check LED pattern (should be slow blink, not solid or error code)
- Check TX16S RSSI (should show signal strength)
- If link lost: See [Link/Telemetry Issues](#linktelemetry-issues)

**Step 4: Check System Errors**
- If LED is blinking error code: Fix that error first
- Power cycle bot to clear transient errors
- Verify watchdog hasn't triggered

**Step 5: Switch Function Test**
- With bot safe (weapon disconnected):
  - Move SA up and down multiple times
  - Should hear/feel switch click
  - Check TX16S channels page: CH6 should change
  - If CH6 doesn't change: SA switch or TX16S config issue

---

### Issue: Erratic Movement / Twitching

**Symptoms**:
- Bot moves unexpectedly
- Motors twitch when sticks centered
- Unstable behavior

**Likely Causes**:

**1. Stick Drift / Deadband Issue**
- TX16S sticks not perfectly centered
- Firmware deadband (0.05 = 5%) may not be enough
- **Solution**: Recalibrate TX16S sticks or increase deadband in firmware

**2. Electrical Noise**
- Poor grounding
- Motor noise coupling into control signals
- **Solution**:
  - Check all ground connections
  - Add capacitors on motor power lines
  - Separate logic and power grounds (star grounding)

**3. Power Brownouts**
- Voltage sag during movement
- Arduino resets briefly
- **Solution**:
  - Check battery voltage under load (should stay >6.8V)
  - Add bulk capacitor (1000µF+) on motor power
  - Ensure motor/logic use separate 5V regulators

**4. L293D Overheating**
- Shield thermally shutting down
- Intermittent operation
- **Solution**:
  - Reduce duty cycle clamp (currently 80%)
  - Add heatsink to L293D chips
  - Improve airflow

---

## Power-On Problems

### No Power LED

**Symptom**: Arduino power LED (near USB jack) doesn't light

**Checklist**:
1. [ ] Battery connected and charged (>10.5V for 3S)
2. [ ] Power switch ON (if present)
3. [ ] Check battery connector polarity (+ and - correct)
4. [ ] Measure battery voltage with multimeter
5. [ ] Check 5V regulator output (should be 5.0V ±0.25V)

**If power LED still off**:
- Arduino or 5V regulator faulty
- Check for shorts on 5V rail
- Verify 5V BEC output (if using external)
- Replace Arduino if needed

---

### LED Pattern Wrong at Power-On

| Observed | Expected | Problem | Fix |
|----------|----------|---------|-----|
| Solid ON | Slow blink | Failsafe active | Check TX off, kill switch, or link |
| Fast blink | Slow blink | Armed at boot (ERROR!) | Power cycle, check firmware |
| Error code | Slow blink | System error detected | See error code section above |
| Off | Slow blink | No power or dead Arduino | Check power, replace Arduino |

---

## Control Issues

### Inverted Controls

**Symptom**: Forward stick goes backward, or rotation reversed

**Cause**: Motor polarity incorrect for wheel/motor configuration

**Fix**: Update motor inversion flags in `config.h`:

```cpp
// Motor polarity inversion flags (adjust during initial testing)
#define MOTOR_FL_INVERTED  false  // Front-Left motor
#define MOTOR_FR_INVERTED  true   // Front-Right motor (example: inverted)
#define MOTOR_RL_INVERTED  false  // Rear-Left motor
#define MOTOR_RR_INVERTED  true   // Rear-Right motor (example: inverted)
```

**Process**:
1. Test each motor direction individually
2. Note which motors run backward
3. Set those flags to `true`
4. Re-upload firmware
5. Test again

---

### Weak / Sluggish Movement

**Symptom**: Bot moves slowly even with sticks at max

**Possible Causes**:

**1. Wrong Drive Mode**
- Currently in **Beginner** mode (SB switch DOWN)
- **Solution**: Move SB to MIDDLE (Normal 80%) or UP (Aggressive 100%)

**2. Battery Low**
- Voltage <7.0V causes reduced performance
- **Solution**: Charge or replace battery

**3. Motor Issues**
- Mechanical binding
- Damaged motors
- **Solution**: Test motors individually, check for obstructions

**4. Duty Cycle Clamp**
- Firmware limits max to 80% for thermal protection
- **Solution**: If needed, increase `MOTOR_DUTY_CLAMP_MAX` in `config.h` (risks overheating)

---

## Weapon Arming Problems

### Weapon Spins at Boot (CRITICAL ERROR)

**Symptom**: Weapon motor runs immediately when battery connected

**THIS SHOULD NEVER HAPPEN** - Indicates serious firmware or hardware fault

**Immediate Action**:
1. **DISCONNECT BATTERY IMMEDIATELY**
2. **DO NOT reconnect until issue resolved**

**Diagnosis**:
- Check weapon ESC wiring (should go to Arduino pin 3)
- Verify ESC is configured for standard PWM (not oneshot/multishot)
- Check `SAFE_WEAPON_US` in `config.h` (should be `WEAPON_ESC_MIN_US` = 1000)
- Re-upload firmware from known-good source

**If problem persists**:
- Arduino pin 3 may be damaged (stuck HIGH)
- ESC may be faulty
- Replace Arduino and/or ESC

---

### Weapon Arms But Won't Spin

**Symptom**:
- LED fast blink (armed)
- Move slider up
- Weapon doesn't spin

**Checklist**:

1. **ESC Connection**:
   - [ ] ESC signal wire → Arduino pin 3
   - [ ] ESC powered from main battery (3S)
   - [ ] ESC ground → Arduino ground

2. **ESC Calibration**:
   - Some ESCs require calibration
   - Consult ESC manual for calibration procedure
   - Typically: Max throttle → power on → min throttle

3. **PWM Signal**:
   - Firmware outputs ~490Hz PWM (not standard 50Hz)
   - Most ESCs tolerate this, but some don't
   - **Solution**: Implement proper 50Hz Timer2 PWM (advanced)

4. **Slider Range**:
   - [ ] Check TX16S: Slider should output 0-100% on CH5
   - [ ] Verify in EdgeTX channels page
   - If range wrong: Adjust slider calibration in TX16S

---

## Link/Telemetry Issues

### No Telemetry on TX16S

**Symptom**: TX16S doesn't show battery voltage or RAM data

**Not Critical**: Bot will work without telemetry (it's optional)

**Diagnosis**:

**Step 1: Receiver RX Pin**
- Not all ExpressLRS receivers have RX pin (bidirectional)
- Check CR8 Nano datasheet
- If no RX pin: Telemetry won't work (expected)

**Step 2: Wiring**
- Arduino TX (pin 1) → Receiver RX pin
- Verify correct pin on receiver (consult datasheet)
- Check wire not broken

**Step 3: Firmware Config**
- Open `config.h`
- Verify `CRSF_TELEMETRY_ENABLED` is `1`
- If `0`: Telemetry disabled, change to `1` and re-upload

**Step 4: TX16S Discovery**
- Model Setup → Telemetry → Discover new sensors
- Wait 10-30 seconds
- "Batt" sensor should appear
- If not: Check wiring and receiver RX pin

---

### Intermittent Link Loss

**Symptom**: Occasional solid LED (failsafe), then recovers

**Causes**:

**1. Low Transmitter Power**
- TX16S battery low
- **Solution**: Charge transmitter

**2. Antenna Damage**
- Receiver antenna bent/broken
- **Solution**: Straighten or replace antenna

**3. RF Interference**
- Other 2.4GHz devices nearby
- Metal arena causing reflections
- **Solution**:
  - Change ExpressLRS frequency (if supported)
  - Reposition receiver antenna
  - Minimize other 2.4GHz sources

**4. Weak Receiver Signal**
- Receiver antenna orientation poor
- Antenna inside metal chassis
- **Solution**:
  - Mount antenna outside chassis
  - Use 90° antenna orientation for best omni pattern

---

## Advanced Diagnostics

### Serial Monitor Debugging

**Not available in competition firmware** (UART used for CRSF)

For debugging:
1. Temporarily disable CRSF in `config.h`
2. Add Serial.begin(115200) in setup()
3. Add debug prints
4. Upload and monitor via USB
5. Remember to re-enable CRSF for competition!

---

### Memory Usage Check

**Symptom**: Strange behavior, crashes, or watchdog resets

**Check RAM usage**:
1. Compile firmware: `pio run`
2. Check output:
   ```
   RAM:   [==        ]  19.3% (used 396 bytes from 2048 bytes)
   ```
3. If >80%: Memory fragmentation or leak (shouldn't happen with static allocation)

**Current firmware**: 396 bytes / 2048 bytes (19.3%) ✓ Healthy

---

### Watchdog Timer Test

**Purpose**: Verify watchdog is working

**WARNING**: This will cause error code 2

**Test Procedure**:
1. Comment out `safety_watchdog_reset()` in main loop
2. Upload firmware
3. Power on → Should see error code 2 (watchdog reset) within 1 second
4. Restore `safety_watchdog_reset()` call
5. Re-upload

If watchdog doesn't trigger: Watchdog not configured correctly (firmware bug)

---

## Hardware Fault Isolation

### Systematic Fault Finding

**Start with minimal system, add components one at a time:**

**Test 1: Arduino Only**
- Disconnect all shields and wires
- Power Arduino via USB
- LED should slow blink
- **Pass**: Arduino OK
- **Fail**: Arduino damaged, replace

**Test 2: Arduino + Receiver**
- Connect receiver (RX to pin 0, TX to pin 1, power, ground)
- Power Arduino via USB
- Turn on TX16S
- LED should slow blink (link OK)
- **Pass**: Receiver and CRSF OK
- **Fail**: Check receiver wiring, binding, or serial config

**Test 3: Add Motor Shield**
- Seat L293D shield on Arduino
- Power via battery (not USB!)
- LED should slow blink
- **Pass**: Shield OK
- **Fail**: Short on shield, check for solder bridges

**Test 4: Add Motors (no load)**
- Connect motors to shield
- Wheels off ground
- Test movement
- **Pass**: Motor control OK
- **Fail**: Check motor connections, polarity

**Test 5: Add Weapon ESC (weapon disconnected)**
- Connect ESC signal wire
- Weapon motor NOT connected
- Arm weapon, move slider
- **Pass**: ESC signal OK
- **Fail**: Check ESC wiring, calibration

**Test 6: Add Weapon Motor (secured)**
- Connect weapon motor to ESC
- Weapon securely mounted/balanced
- Arm and test at low speed
- **Pass**: Full system OK
- **Fail**: Check weapon mechanical, ESC power

**Test 7: Add Self-Right Servo**
- Connect servo to pin 11
- Test with SH button
- **Pass**: All systems operational!
- **Fail**: Check servo wiring, power

---

## When to Reflash Firmware

**Reflash if**:
- Persistent error codes that don't clear
- Watchdog resets with no obvious cause
- Controls completely non-responsive
- After modifying config.h settings
- Updating to newer firmware version

**How to reflash**:
1. Download latest firmware from GitHub
2. Connect Arduino via USB
3. Run: `pio run --target upload`
4. Wait for "SUCCESS"
5. Disconnect USB, reconnect battery
6. Test functionality

---

## Contact & Support

**Firmware Issues**:
- GitHub: https://github.com/blakelton/UpVote_Arduino_BattleBot
- Check `docs/ai_eval/FINAL_REPORT.md` for known issues

**Hardware Issues**:
- Check component datasheets
- Verify wiring against `docs/hardware/pin_assignments.md`

**Competition Rules**:
- Consult your competition organizer
- Follow safety protocols

---

## Appendix: Quick Reference Tables

### Pin Assignment Reference

| Pin | Function | Component |
|-----|----------|-----------|
| 0 | UART RX | CRSF Receiver TX |
| 1 | UART TX | CRSF Receiver RX (telemetry) |
| 3 | PWM | Weapon ESC signal |
| 4 | Shift Register Latch | L293D shield (motor direction) |
| 5 | PWM | Motor FL (front-left) |
| 6 | PWM | Motor FR (front-right) |
| 7 | Shift Register Clock | L293D shield (motor direction) |
| 8 | Shift Register Data | L293D shield (motor direction) |
| 9 | PWM | Motor RL (rear-left) |
| 10 | PWM | Motor RR (rear-right) |
| 11 | PWM | Self-right servo signal |
| 12 | Shift Register Enable | L293D shield (motor enable) |
| 13 | LED | Status indicator |

### Component Specifications

| Component | Spec | Notes |
|-----------|------|-------|
| **Arduino UNO** | ATmega328P | 16MHz, 2KB RAM, 32KB Flash |
| **Battery** | 3S LiPo | 11.1V nominal, 12.6V full |
| **Receiver** | CR8 Nano | ExpressLRS 2.4GHz |
| **Transmitter** | RadioMaster TX16S | EdgeTX firmware |
| **Motor Shield** | L293D | Dual H-bridge, 600mA per channel |
| **Weapon ESC** | Standard hobby ESC | 1000-2000µs PWM |
| **Servo** | Standard hobby servo | 500-2500µs PWM |

---

**Document Version**: 1.0
**Last Updated**: 2025-12-26

*For additional help, see [OPERATOR_GUIDE.md](OPERATOR_GUIDE.md) and [LED_STATUS_REFERENCE.md](LED_STATUS_REFERENCE.md)*
