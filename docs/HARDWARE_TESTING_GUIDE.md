# UpVote Battlebot - Hardware Testing Guide

**Version**: 1.0
**Firmware**: Phases 1-6 Complete
**Last Updated**: 2025-12-26

---

## Purpose

This guide provides **incremental testing procedures** for validating firmware functionality as you build the hardware. Each test builds on previous tests, ensuring systematic verification of all features.

**Test Philosophy**:
- Test early, test often
- Validate each subsystem before integration
- Document failures for troubleshooting
- Never skip safety checks

---

## Table of Contents

1. [Required Equipment](#required-equipment)
2. [Test Environment Setup](#test-environment-setup)
3. [Test Sequence Overview](#test-sequence-overview)
4. [Test 1: Arduino Power-On](#test-1-arduino-power-on)
5. [Test 2: Firmware Upload](#test-2-firmware-upload)
6. [Test 3: LED Diagnostics](#test-3-led-diagnostics)
7. [Test 4: Receiver Power & Binding](#test-4-receiver-power--binding)
8. [Test 5: CRSF Link Verification](#test-5-crsf-link-verification)
9. [Test 6: Telemetry Validation](#test-6-telemetry-validation)
10. [Test 7: Single Drive Motor](#test-7-single-drive-motor)
11. [Test 8: Four-Motor Holonomic Drive](#test-8-four-motor-holonomic-drive)
12. [Test 9: Weapon Motor Control](#test-9-weapon-motor-control)
13. [Test 10: Self-Right Servo](#test-10-self-right-servo)
14. [Test 11: Safety Interlocks](#test-11-safety-interlocks)
15. [Test 12: Stress Testing](#test-12-stress-testing)
16. [Test Results Log Template](#test-results-log-template)

---

## Required Equipment

### Hardware
- ☐ Arduino UNO (ATmega328P)
- ☐ USB cable (Type A to Type B for Arduino programming)
- ☐ CR8 Nano ExpressLRS receiver
- ☐ RadioMaster TX16S transmitter (bound to receiver)
- ☐ 2S LiPo battery (7.4V, fully charged to 8.4V)
- ☐ 5V BEC (3A minimum, 5A recommended)
- ☐ 4× Brushed drive motors with ESCs
- ☐ 1× Weapon motor with ESC
- ☐ 1× Standard servo (5V)
- ☐ Jumper wires (M-M, M-F) for breadboard testing
- ☐ Breadboard (optional, for early testing)

### Tools
- ☐ Multimeter (voltage, continuity, resistance)
- ☐ Wire strippers
- ☐ Soldering iron and solder
- ☐ Heat shrink tubing
- ☐ Small screwdriver set
- ☐ Electrical tape
- ☐ Zip ties for cable management

### Software
- ☐ PlatformIO IDE (VS Code extension)
- ☐ Arduino drivers installed
- ☐ UpVote firmware source code (Phases 1-6)
- ☐ TX16S configured with UpVote model

### Safety Equipment
- ☐ Safety glasses
- ☐ Fire extinguisher (for LiPo safety)
- ☐ LiPo charging bag
- ☐ Non-conductive work surface

### Documentation
- ☐ [WIRING_DIAGRAM.md](WIRING_DIAGRAM.md) - Printed reference
- ☐ [LED_STATUS_REFERENCE.md](LED_STATUS_REFERENCE.md) - LED pattern guide
- ☐ [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Diagnostic procedures
- ☐ This testing guide (printed)

---

## Test Environment Setup

### Work Area
1. **Non-conductive surface** (wood table, plastic mat)
2. **Good lighting** (see LED patterns clearly)
3. **Clear area** (no clutter, safe for testing)
4. **Fire safety** (LiPo charging bag nearby, extinguisher accessible)

### Battery Safety
- ☐ Charge battery in LiPo bag
- ☐ Verify voltage: 8.4V ± 0.1V (full charge)
- ☐ Inspect battery: No puffing, no damage
- ☐ Check connector polarity (Red = +, Black = -)

### Transmitter Setup
- ☐ TX16S powered on
- ☐ "UpVote" model selected
- ☐ Battery > 50%
- ☐ All switches in safe positions:
  - **SA** (Arm): ↓ DOWN (disarmed)
  - **SD** (Kill): ↓ DOWN (inactive)
  - **SB** (Drive Mode): ↔ MIDDLE (normal)
  - **Slider** (Weapon): ↓ BOTTOM (0%)

---

## Test Sequence Overview

| # | Test | Duration | Pass Criteria | Prerequisites |
|---|------|----------|---------------|---------------|
| 1 | Arduino Power-On | 5 min | Power LED lit | USB cable |
| 2 | Firmware Upload | 10 min | Upload success, no errors | Test 1 |
| 3 | LED Diagnostics | 5 min | Slow blink (1 Hz) | Test 2 |
| 4 | Receiver Power & Binding | 10 min | RX green LED blinks | Test 3, BEC |
| 5 | CRSF Link Verification | 10 min | LED stays slow blink | Test 4, wiring |
| 6 | Telemetry Validation | 10 min | TX shows voltage & RAM | Test 5 |
| 7 | Single Drive Motor | 15 min | Motor responds to stick | Test 5, 1 ESC |
| 8 | Four-Motor Holonomic | 20 min | All directions work | Test 7, 4 ESCs |
| 9 | Weapon Motor Control | 15 min | Weapon spins when armed | Test 5, weapon ESC |
| 10 | Self-Right Servo | 10 min | Servo extends/retracts | Test 5, servo |
| 11 | Safety Interlocks | 15 min | All safety features work | Tests 8, 9, 10 |
| 12 | Stress Testing | 30 min | Stable under load | Test 11 |

**Total Estimated Time**: 2.5-3 hours (can spread over multiple sessions)

---

## Test 1: Arduino Power-On

**Goal**: Verify Arduino hardware functional

**Required**:
- Arduino UNO
- USB cable
- Computer

**Procedure**:
1. Connect Arduino to computer via USB
2. Observe Arduino board

**Expected Results**:
- ✅ Green "ON" power LED lights up
- ✅ Orange "L" LED may blink (bootloader)
- ✅ Computer recognizes Arduino (check Device Manager on Windows)

**Pass Criteria**:
- [ ] Power LED is ON
- [ ] Computer detects Arduino (COM port assigned)

**Troubleshooting**:
- **No power LED**: Try different USB cable, different USB port
- **Not detected**: Install Arduino USB drivers (CH340 or ATmega16U2)

**Notes**:
```
Date: ____________
Time: ____________
Result: PASS / FAIL
Arduino recognized as: COM___
Notes: _________________________________
```

---

## Test 2: Firmware Upload

**Goal**: Flash Phase 1-6 firmware to Arduino

**Required**:
- Arduino connected via USB
- PlatformIO installed
- Firmware source code

**Procedure**:
1. Open project in VS Code with PlatformIO
2. Open terminal in VS Code
3. Run: `platformio run --target upload`
4. Wait for upload to complete

**Expected Results**:
```
Uploading .pio\build\uno\firmware.hex
avrdude: 9370 bytes of flash written
avrdude: verifying ...
avrdude: 9370 bytes of flash verified

SUCCESS
```

**Pass Criteria**:
- [ ] Upload completes with "SUCCESS"
- [ ] Memory usage shown:
  - RAM: ~396 bytes (19.3%)
  - Flash: ~9370 bytes (29.0%)

**Troubleshooting**:
- **Upload failed**: Check COM port, press reset button before upload
- **Verification error**: Re-upload, try different USB cable

**Notes**:
```
Date: ____________
Time: ____________
Result: PASS / FAIL
RAM: _____ bytes (_____%)
Flash: _____ bytes (_____%)
Upload time: _____ seconds
Notes: _________________________________
```

---

## Test 3: LED Diagnostics

**Goal**: Verify LED status indicator working (Phase 1)

**Required**:
- Firmware uploaded (Test 2)
- Arduino powered via USB

**Procedure**:
1. Arduino connected via USB (no battery yet)
2. Observe built-in LED (pin 13, yellow LED on board)
3. Wait 5 seconds for pattern to stabilize

**Expected Results**:
- ✅ LED blinks slowly: 500ms ON, 500ms OFF (1 Hz)
- ✅ Pattern is consistent and regular

**Why Slow Blink?**:
- No battery = no CRSF receiver = link_ok is false
- But error is ERR_NONE (no actual error)
- Status = SAFE (disarmed, no link yet)
- LED pattern = Slow blink (1 Hz)

**Pass Criteria**:
- [ ] LED blinks at 1 Hz (1 blink per second)
- [ ] Pattern is stable (not erratic)

**Troubleshooting**:
- **Solid ON**: Link detected (shouldn't happen on USB power alone)
- **Fast blink (5 Hz)**: Armed state (shouldn't happen, no receiver)
- **Error pattern (N blinks)**: See [LED_STATUS_REFERENCE.md](LED_STATUS_REFERENCE.md)
- **No blink**: Re-upload firmware, check LED (may be damaged)

**Notes**:
```
Date: ____________
Time: ____________
Result: PASS / FAIL
LED Pattern: _________________________________
Blink frequency: _____ Hz
Notes: _________________________________
```

---

## Test 4: Receiver Power & Binding

**Goal**: Power CR8 Nano receiver and verify binding to TX16S

**Required**:
- BEC (5V output)
- CR8 Nano receiver
- 2S LiPo battery (charged)
- TX16S transmitter (powered on)
- Jumper wires

**Safety Check**:
- ☐ Battery voltage verified: 8.4V ± 0.1V
- ☐ BEC polarity verified with multimeter
- ☐ Transmitter on with correct model selected

**Procedure**:

**Step 1: BEC Connection**
1. **Disconnect Arduino from USB** (never power from USB + battery simultaneously)
2. Connect battery to BEC input:
   - Battery (+) → BEC (+)
   - Battery (-) → BEC (-)
3. Measure BEC output with multimeter: Should be **5.0V ± 0.2V**

**Step 2: Receiver Power**
1. Connect BEC 5V output to CR8 Nano:
   - BEC 5V → CR8 red wire (any channel pin)
   - BEC GND → CR8 black wire (any channel pin)
2. Observe CR8 green LED

**Expected Results**:
- ✅ CR8 green LED blinks (indicates receiver powered)
- ✅ Blink pattern depends on binding:
  - **Slow blink (~1 Hz)**: Not bound to transmitter
  - **Fast blink (~5-10 Hz)**: Bound to transmitter, link active

**Step 3: Binding (if needed)**
1. If CR8 slow blinking (not bound):
   - Press bind button on CR8 while powering on
   - On TX16S: System → ExpressLRS → Bind
   - Wait for CR8 to fast blink (bound)
2. Verify TX16S shows "Connected" or telemetry values

**Pass Criteria**:
- [ ] BEC outputs 5.0V ± 0.2V
- [ ] CR8 green LED blinks (powered)
- [ ] CR8 fast blinks (bound to TX16S)
- [ ] TX16S shows "Connected"

**Troubleshooting**:
- **BEC not 5V**: Check battery voltage, BEC wiring, replace BEC
- **CR8 no LED**: Check power wiring, try different power source
- **CR8 won't bind**: Reset binding (hold bind button 3+ seconds), retry

**Notes**:
```
Date: ____________
Time: ____________
Result: PASS / FAIL
BEC Output: _____ V
CR8 LED: Slow blink / Fast blink / No blink
Binding: Already bound / New binding required
TX16S shows: _________________________________
Notes: _________________________________
```

---

## Test 5: CRSF Link Verification

**Goal**: Verify CRSF communication between CR8 and Arduino (Phase 2)

**Required**:
- Test 4 complete (receiver powered and bound)
- Arduino UNO
- Jumper wires

**Safety Check**:
- ☐ TX16S powered on, correct model selected
- ☐ All switches in safe positions (SA DOWN, SD DOWN, slider BOTTOM)

**Wiring**:
```
BEC 5V → Arduino 5V pin
BEC GND → Arduino GND pin
CR8 TX → Arduino D0 (RX)
CR8 RX → Arduino D1 (TX) [optional, for telemetry]
```

**Procedure**:

**Step 1: Power Arduino from Battery**
1. Wire BEC to Arduino as shown above
2. Wire CR8 UART to Arduino (TX → RX crossover)
3. Power on (battery connected to BEC)
4. **Wait 3 seconds** for link to establish

**Step 2: Observe LED**
1. Watch Arduino built-in LED (pin 13)
2. Note the pattern

**Expected Results**:
- ✅ LED slow blinks (1 Hz) - SAFE mode
- ✅ Pattern is stable (not changing randomly)
- ✅ TX16S shows telemetry (if telemetry enabled)

**Why Slow Blink?**:
- Link established (link_ok = true)
- Not armed (arm_switch = false, SA DOWN)
- No errors (error = ERR_NONE)
- Status = SAFE
- LED = Slow blink (1 Hz)

**Step 3: Test Kill Switch**
1. Move **SD switch UP** (kill switch active)
2. Observe LED → Should go **SOLID ON** (FAILSAFE)
3. Move **SD switch DOWN** (kill inactive)
4. Observe LED → Should return to **slow blink**

**Pass Criteria**:
- [ ] LED slow blinks with link active (SA DOWN, SD DOWN)
- [ ] LED solid ON when SD switch UP (kill active)
- [ ] LED returns to slow blink when SD DOWN
- [ ] Pattern is stable (no errors)

**Troubleshooting**:
- **LED solid ON (SD DOWN)**: No CRSF link
  - Check TX16S is on
  - Check CR8 TX → Arduino RX wiring (crossover!)
  - Verify baudrate 420000 in firmware
  - Check CR8 green LED fast blinks (bound)
- **LED error pattern (N blinks)**: See [LED_STATUS_REFERENCE.md](LED_STATUS_REFERENCE.md)
  - 3 blinks = CRSF timeout (no packets received)
  - 4 blinks = CRSF CRC error (bad packets)
- **LED fast blink**: Bot thinks it's armed (check SA switch DOWN)

**Notes**:
```
Date: ____________
Time: ____________
Result: PASS / FAIL
LED pattern (SD DOWN): _________________________________
LED pattern (SD UP): _________________________________
Link stable: YES / NO
Errors observed: _________________________________
Notes: _________________________________
```

---

## Test 6: Telemetry Validation

**Goal**: Verify CRSF telemetry from Arduino to TX16S (Phase 2.5)

**Required**:
- Test 5 complete (CRSF link working)
- Telemetry enabled in firmware (check `config.h`)

**Verify Telemetry Enabled**:
1. Open `include/config.h`
2. Find: `#define CRSF_TELEMETRY_ENABLED 1`
3. If `0`, change to `1`, re-upload firmware

**Procedure**:

**Step 1: Check TX16S Telemetry Screen**
1. On TX16S, navigate to telemetry screen:
   - Long press **SYS** (System)
   - Select **Telemetry**
   - Or tap **TELE** page button
2. Wait 2-3 seconds for telemetry to arrive

**Step 2: Verify Telemetry Values**
1. Check displayed values:
   - **Battery Voltage**: Should show ~7.4V (or actual battery voltage)
   - **Fuel/Remaining %**: Should show ~80-85% (free RAM percentage)
   - **Current**: 0A (not measured)
   - **Capacity Used**: Shows error code (should be 0 for no error)

**Expected Results**:
- ✅ Battery voltage displayed (7.4V nominal, 8.4V fully charged)
- ✅ Fuel/Remaining shows 80-85% (free RAM ≈ 1650/2048 bytes = 80%)
- ✅ Values update every ~1 second

**Pass Criteria**:
- [ ] TX16S shows battery voltage
- [ ] Voltage matches actual battery (± 0.5V)
- [ ] Free RAM percentage shown (80-85% expected)
- [ ] Values update periodically (~1 Hz)

**Troubleshooting**:
- **No telemetry**:
  - Check CR8 RX → Arduino TX wiring
  - Verify telemetry enabled in firmware
  - Check TX16S telemetry screen configured
- **Wrong voltage**:
  - If no battery monitor (default): Shows 7.4V nominal (hardcoded)
  - To monitor real voltage: Add voltage divider to A0 (see WIRING_DIAGRAM.md)
- **RAM % incorrect**: Should be ~80-85% free

**Notes**:
```
Date: ____________
Time: ____________
Result: PASS / FAIL
Battery voltage shown: _____ V
Actual battery voltage: _____ V
Free RAM %: _____ %
Telemetry update rate: _____ Hz
Notes: _________________________________
```

---

## Test 7: Single Drive Motor

**Goal**: Verify one drive motor responds to transmitter stick input (Phase 4)

**Required**:
- Test 5 complete (CRSF link working)
- 1× Drive motor with ESC
- Jumper wires or ESC servo connector

**Safety**:
- ☐ Motor not installed in bot (loose on bench)
- ☐ No weapon installed
- ☐ SA switch DOWN (disarmed - motor won't spin yet)

**Wiring** (Front-Left motor example):
```
ESC Power (+) → Battery 7.4V rail
ESC Ground (-) → Ground rail (common with Arduino GND)
ESC Signal → Arduino D3 (PIN_MOTOR_FL)
ESC Output → Motor (2 wires)
```

**Procedure**:

**Step 1: Power On**
1. Wire ESC as shown above
2. Power on system (battery → BEC → Arduino, CRSF link active)
3. Verify LED slow blinks (SAFE mode)

**Step 2: Test Motor Response**
1. Move **right stick** (roll/pitch controls):
   - Forward (Y+): Motor should spin forward
   - Backward (Y-): Motor should spin backward
   - Right (X+): Motor should spin (FL motor increases with right)
   - Left (X-): Motor should spin opposite
2. Try **left stick** (yaw control):
   - Right (X+): Motor should spin for rotation
   - Left (X-): Motor should spin opposite

**Expected Results**:
- ✅ Motor responds to stick input
- ✅ Motor speed proportional to stick deflection
- ✅ Motor stops when stick centered (deadband working)
- ✅ Slew rate limiting visible (gradual acceleration, not instant)

**Why Motor Spins (Disarmed)**:
- Weapon arming only affects weapon motor (pin 10)
- Drive motors (pins 3, 5, 6, 9) always respond to sticks
- This is correct behavior (drive always available for positioning)

**Pass Criteria**:
- [ ] Motor spins when stick moved
- [ ] Motor speed proportional to stick
- [ ] Motor stops when stick centered
- [ ] Direction correct (forward/back/rotate)
- [ ] No jittering or twitching

**Troubleshooting**:
- **No response**:
  - Check ESC power (7.4V)
  - Check ESC signal wire to Arduino D3
  - Check ESC ground to Arduino GND
  - Verify stick moves on TX16S (check screen)
  - Calibrate ESC if needed (see ESC manual)
- **Wrong direction**: Swap motor wires
- **Jittering**: Check ground connections, deadband setting

**Notes**:
```
Date: ____________
Time: ____________
Result: PASS / FAIL
Motor tested: FL / FR / RL / RR
Arduino pin: D___
Direction correct: YES / NO
Speed control: Smooth / Jerky
Notes: _________________________________
```

---

## Test 8: Four-Motor Holonomic Drive

**Goal**: Verify all four drive motors work together (Phase 3 + 4)

**Required**:
- Test 7 complete (single motor works)
- 4× Drive motors with ESCs
- Bot chassis (motors installed in holonomic configuration)
- Blocks to lift wheels off ground

**Safety**:
- ☐ Bot on blocks (wheels off ground)
- ☐ No weapon installed
- ☐ SA switch DOWN (disarmed)
- ☐ SB switch MIDDLE (Normal drive mode)

**Wiring** (all 4 motors):
```
ESC FL Signal → Arduino D3
ESC FR Signal → Arduino D5
ESC RL Signal → Arduino D6
ESC RR Signal → Arduino D9
All ESC power → Battery 7.4V rail
All ESC ground → Ground rail (common with Arduino)
```

**Procedure**:

**Step 1: Power On**
1. Wire all 4 ESCs as shown
2. Place bot on blocks (wheels off ground)
3. Power on, verify LED slow blinks

**Step 2: Test Forward/Backward (Pitch)**
1. Move **right stick forward** (Y+)
2. All 4 wheels should spin **forward** (same direction)
3. Move **right stick backward** (Y-)
4. All 4 wheels should spin **backward**

**Step 3: Test Strafe Left/Right (Roll)**
1. Move **right stick right** (X+)
2. Wheels should spin to **strafe right**:
   - FL: forward, FR: backward, RL: backward, RR: forward
3. Move **right stick left** (X-)
4. Wheels should spin to **strafe left** (opposite of above)

**Step 4: Test Rotation (Yaw)**
1. Move **left stick right** (yaw+)
2. Wheels should spin to **rotate clockwise**:
   - FL: forward, FR: backward, RL: forward, RR: backward
3. Move **left stick left** (yaw-)
4. Wheels should spin to **rotate counterclockwise** (opposite)

**Step 5: Test Combined Movement**
1. Move **right stick diagonally** (forward + right)
2. Bot should move forward while strafing right
3. Add **left stick** (rotation)
4. All 3 axes should combine smoothly

**Expected Results** (Holonomic Mixing):
```
Movement | FL       | FR       | RL       | RR
---------|----------|----------|----------|----------
Forward  | Forward  | Forward  | Forward  | Forward
Backward | Backward | Backward | Backward | Backward
Right    | Forward  | Backward | Backward | Forward
Left     | Backward | Forward  | Forward  | Backward
CW Rot   | Forward  | Backward | Forward  | Backward
CCW Rot  | Backward | Forward  | Backward | Forward
```

**Pass Criteria**:
- [ ] All 4 motors respond to sticks
- [ ] Forward/backward: All wheels same direction
- [ ] Strafe: Correct holonomic pattern
- [ ] Rotation: Correct spin pattern
- [ ] Combined movements smooth
- [ ] No motor runs backward when it should be forward

**Troubleshooting**:
- **One motor wrong direction**: Swap that motor's wires
- **One motor not responding**: Check that ESC signal wire, power, ground
- **Incorrect mixing**: Verify pin assignments in config.h match physical layout
- **Weak rotation**: Yaw scaled to 70% (configurable in HOLONOMIC_YAW_SCALE)

**Motor Layout Verification**:
```
Front of bot
     ↑
  FL   FR
  RL   RR
```
- FL = Front-Left (pin 3)
- FR = Front-Right (pin 5)
- RL = Rear-Left (pin 6)
- RR = Rear-Right (pin 9)

**Notes**:
```
Date: ____________
Time: ____________
Result: PASS / FAIL
Forward/Back: PASS / FAIL
Strafe: PASS / FAIL
Rotation: PASS / FAIL
Combined: PASS / FAIL
Motor directions corrected: _________________________________
Notes: _________________________________
```

---

## Test 9: Weapon Motor Control

**Goal**: Verify weapon arming state machine and motor control (Phase 5)

**Required**:
- Test 5 complete (CRSF link working)
- Weapon motor with ESC
- **NO weapon blade installed** (bare motor shaft only)

**⚠️ SAFETY CRITICAL**:
- ☐ NO weapon blade/disk/spinner installed (motor shaft bare)
- ☐ Bot secured (cannot move if motor spins)
- ☐ Clear area (no one near bot)
- ☐ Kill switch (SD) ready to use

**Wiring**:
```
Weapon ESC Signal → Arduino D10 (PIN_WEAPON_ESC)
Weapon ESC Power → Battery 7.4V rail
Weapon ESC Ground → Ground rail (common with Arduino)
Weapon ESC Output → Weapon motor
```

**Procedure**:

**Step 1: Verify Disarmed State**
1. Power on system
2. **SA switch DOWN** (disarmed)
3. Move **weapon slider** to 100%
4. Weapon should **NOT spin** (disarmed)

**Step 2: Arming Sequence**
1. Move **weapon slider to 0%** (CRITICAL: must be at bottom)
2. Move **SA switch UP** (arm)
3. Observe LED → Should change to **fast blink (5 Hz)**
4. LED fast blink = ARMED ✓

**Step 3: Weapon Spin Test**
1. Slowly move **weapon slider** from 0% → 10%
2. Weapon motor should start spinning (slow)
3. Move slider to 50%
4. Weapon motor should spin faster
5. Move slider to 100%
6. Weapon motor should spin at full speed

**Step 4: Weapon Slew Rate**
1. Weapon at 100%, observe spin speed
2. Quickly move slider to 0%
3. Weapon should **NOT stop instantly**
4. Should gradually slow down over ~2 seconds (slew rate limiting)

**Step 5: Disarm Test**
1. With weapon at 50%, move **SA switch DOWN** (disarm)
2. Weapon should **immediately stop** (disarmed)
3. LED should return to **slow blink**

**Step 6: Re-Arm Hysteresis Test**
1. Move **weapon slider to 50%** (not at 0%)
2. Try to arm: **SA switch UP**
3. Weapon should **NOT arm** (LED stays slow blink)
4. Move **slider below 10%** (hysteresis threshold)
5. Now arm: **SA switch UP**
6. Weapon should arm (LED fast blink)

**Expected Results**:
- ✅ Weapon does NOT spin when disarmed (SA DOWN)
- ✅ Arming requires slider at 0-3%
- ✅ LED changes to fast blink when armed
- ✅ Weapon spins proportional to slider (0-100%)
- ✅ Slew rate limits acceleration (~2 sec 0→100%)
- ✅ Disarming immediately stops weapon
- ✅ Re-arming requires slider < 10% (hysteresis)

**Pass Criteria**:
- [ ] Weapon does not spin when disarmed
- [ ] Arming only works when slider at 0-3%
- [ ] LED fast blinks when armed
- [ ] Weapon speed controlled by slider
- [ ] Slew rate limiting works (~2 sec ramp)
- [ ] Disarm immediately stops weapon
- [ ] Re-arm requires slider < 10%

**Troubleshooting**:
- **Weapon spins when disarmed**: CRITICAL BUG, do not use!
  - Check firmware (should be Phases 1-6)
  - Verify SA switch wired to correct channel (CH6)
- **Won't arm**:
  - Check slider at 0% (bottom)
  - Check SA switch UP
  - Check SD switch DOWN (kill inactive)
  - Check link OK (LED not solid ON)
- **No speed control**: Check weapon slider channel (CH5 in firmware)
- **Instant stop (no slew)**: Check WEAPON_SLEW_RATE_MAX in config.h

**Notes**:
```
Date: ____________
Time: ____________
Result: PASS / FAIL
Arming works: YES / NO
Speed control: Smooth / Instant
Slew rate: ~_____ seconds 0→100%
Disarm immediate: YES / NO
Re-arm hysteresis: YES / NO
Safety interlocks: PASS / FAIL
Notes: _________________________________
```

---

## Test 10: Self-Right Servo

**Goal**: Verify self-right servo control (Phase 6)

**Required**:
- Test 5 complete (CRSF link working)
- Standard 5V hobby servo
- Servo arm/horn attached

**Wiring**:
```
Servo Signal (white/yellow) → Arduino D11 (PIN_SELFRIGHT_SERVO)
Servo Power (red) → Arduino 5V pin (or separate BEC if high-torque servo)
Servo Ground (black/brown) → Arduino GND
```

**Procedure**:

**Step 1: Neutral Position**
1. Power on system
2. **SH button** (self-right) released
3. **SD switch DOWN** (kill inactive)
4. Servo should be at **neutral position** (~1500 µs)

**Step 2: Extend Test**
1. Press and hold **SH button** (self-right switch active)
2. Servo should **slowly extend** (move to ~2300 µs)
3. Extension should take ~400ms (slew rate limited)
4. Hold SH for 2 seconds
5. Servo should reach full extend and hold

**Step 3: Retract Test**
1. Release **SH button**
2. Servo should **slowly retract** to neutral (~1500 µs)
3. Retraction should take ~400ms
4. Servo should hold neutral position

**Step 4: Kill Switch Override**
1. Press and hold **SH button** (servo extending)
2. While extending, activate **SD switch UP** (kill)
3. Servo should **immediately** return to neutral
4. LED should go solid ON (FAILSAFE)
5. Release SH and move SD DOWN
6. System returns to normal

**Step 5: Rapid Cycling Test (Brownout Check)**
1. Rapidly press/release SH button 5 times
2. System should remain stable (no resets)
3. If Arduino resets: Servo drawing too much current

**Expected Results**:
- ✅ Servo at neutral when SH released
- ✅ Servo extends slowly when SH pressed (~400ms)
- ✅ Servo retracts slowly when SH released (~400ms)
- ✅ Kill switch (SD) immediately returns servo to neutral
- ✅ No brownouts or resets during servo movement

**Pass Criteria**:
- [ ] Servo responds to SH button
- [ ] Extension smooth and gradual (~400ms)
- [ ] Retraction smooth and gradual (~400ms)
- [ ] Kill switch overrides servo (immediate neutral)
- [ ] No system resets during servo use

**Troubleshooting**:
- **No servo response**:
  - Check servo power (5V)
  - Check signal wire to D11
  - Check ground connection
  - Test servo with servo tester
- **Instant movement (no slew)**:
  - Check SERVO_SLEW_RATE_MAX in config.h (should be 5)
- **Wrong direction**: Reverse servo endpoints in config.h:
  - Swap SERVO_ENDPOINT_RETRACT and SERVO_ENDPOINT_EXTEND values
- **Arduino resets when servo moves**:
  - Servo drawing too much current (> 500mA)
  - Use separate 5V BEC for servo
  - Or use smaller servo

**Servo Endpoint Adjustment** (if needed):
```cpp
// In include/config.h
#define SERVO_ENDPOINT_RETRACT  700   // Retracted (neutral)
#define SERVO_ENDPOINT_EXTEND  2300   // Extended (flip position)
#define SERVO_NEUTRAL_US       1500   // Neutral position
```
Adjust these values if servo doesn't reach desired positions.

**Notes**:
```
Date: ____________
Time: ____________
Result: PASS / FAIL
Servo responds: YES / NO
Extension time: ~_____ ms
Retraction time: ~_____ ms
Kill override: YES / NO
Brownouts: YES / NO
Endpoint adjustment needed: YES / NO
Notes: _________________________________
```

---

## Test 11: Safety Interlocks

**Goal**: Verify all safety features work as designed (Phases 1-6)

**Required**:
- All previous tests complete
- All systems integrated

**Safety Interlocks to Test**:
1. Weapon arming preconditions
2. Kill switch override
3. Link loss failsafe
4. Error handling

### Test 11.1: Weapon Arming Preconditions

**Goal**: Verify weapon ONLY arms when ALL conditions met

**Procedure**:

**Test A: Arm with Kill Active**
1. **SD switch UP** (kill active)
2. **SA switch UP** (try to arm)
3. **Slider at 0%**
4. Result: Should **NOT arm** (LED stays solid ON)

**Test B: Arm with Slider High**
1. **SD switch DOWN** (kill inactive)
2. **Weapon slider at 50%** (not at 0%)
3. **SA switch UP** (try to arm)
4. Result: Should **NOT arm** (LED stays slow blink)

**Test C: Arm with Link Lost**
1. Turn OFF TX16S (link loss)
2. **SA switch UP** (try to arm)
3. Result: Should **NOT arm** (LED solid ON, failsafe)

**Test D: Successful Arm (All Conditions Met)**
1. TX16S ON, correct model
2. **SD switch DOWN** (kill inactive)
3. **Weapon slider at 0%** (bottom)
4. **SA switch UP** (arm)
5. Result: Should **ARM** (LED fast blink)

**Pass Criteria**:
- [ ] Cannot arm with kill switch active
- [ ] Cannot arm with slider > 3%
- [ ] Cannot arm with link lost
- [ ] CAN arm when all conditions met

### Test 11.2: Kill Switch Override

**Goal**: Verify kill switch (SD) immediately disarms all outputs

**Procedure**:
1. Arm bot (SA UP, slider 0%, LED fast blink)
2. Move weapon slider to 50% (weapon spinning)
3. Move drive sticks (bot moving)
4. Press SH button (servo extending)
5. **Activate SD switch UP** (KILL)
6. **Immediately observe**:
   - Weapon motor stops ✓
   - Drive motors stop ✓
   - Servo returns to neutral ✓
   - LED goes solid ON (FAILSAFE) ✓

**Pass Criteria**:
- [ ] Kill switch stops weapon immediately
- [ ] Kill switch stops drive motors immediately
- [ ] Kill switch returns servo to neutral immediately
- [ ] LED goes solid ON (FAILSAFE mode)
- [ ] System recovers when SD DOWN (returns to SAFE)

### Test 11.3: Link Loss Failsafe

**Goal**: Verify link loss triggers failsafe (>200ms timeout)

**Procedure**:
1. Bot armed and operating normally
2. Turn OFF TX16S transmitter (simulate link loss)
3. **Wait 1 second**
4. **Observe**:
   - Weapon motor stops ✓
   - Drive motors stop ✓
   - Servo returns to neutral ✓
   - LED goes solid ON (FAILSAFE) ✓
5. Turn ON TX16S
6. System should recover (LED returns to slow blink)

**Pass Criteria**:
- [ ] Link loss stops all outputs within 1 second
- [ ] LED goes solid ON (FAILSAFE)
- [ ] System recovers when link restored
- [ ] No errors after recovery

### Test 11.4: Error Handling

**Goal**: Verify error LED patterns display correctly

This test requires intentionally causing errors (advanced).

**Optional Tests**:
- **CRC Error**: Induce interference (bring Wi-Fi router close)
  - Should see 4-blink pattern if CRC errors occur
- **Loop Overrun**: Should not occur in normal operation
  - If seen (1 blink pattern): Report to developers

**Pass Criteria**:
- [ ] Errors display correct LED pattern (see LED_STATUS_REFERENCE.md)
- [ ] Errors clear when condition resolved

**Notes**:
```
Date: ____________
Time: ____________
Result: PASS / FAIL

11.1 Arming Preconditions: PASS / FAIL
  - Kill active blocks arm: PASS / FAIL
  - Slider high blocks arm: PASS / FAIL
  - Link loss blocks arm: PASS / FAIL
  - Correct conditions allow arm: PASS / FAIL

11.2 Kill Switch: PASS / FAIL
  - Stops weapon: PASS / FAIL
  - Stops drive: PASS / FAIL
  - Returns servo: PASS / FAIL
  - LED solid ON: PASS / FAIL

11.3 Link Loss: PASS / FAIL
  - Failsafe activates: PASS / FAIL
  - Stops all outputs: PASS / FAIL
  - Recovers on link restore: PASS / FAIL

11.4 Error Handling: PASS / FAIL / SKIPPED

Notes: _________________________________
```

---

## Test 12: Stress Testing

**Goal**: Verify system stability under continuous operation and load

**Required**:
- All previous tests complete
- Fully assembled bot (on blocks)
- Charged battery

**Procedure**:

### Test 12.1: Continuous Operation (5 minutes)

1. Power on bot
2. Arm weapon (SA UP, slider 50%)
3. Operate drive continuously:
   - Forward/back
   - Strafe
   - Rotation
   - Combined movements
4. Occasionally use self-right (SH button)
5. Run for **5 minutes continuous**

**Monitor**:
- [ ] LED pattern stable (fast blink, no errors)
- [ ] No unexpected resets
- [ ] Battery voltage (check telemetry)
- [ ] BEC temperature (should be warm, not hot)
- [ ] Arduino temperature (should be cool)

**Pass Criteria**:
- [ ] System runs 5+ minutes without reset
- [ ] No error LED patterns
- [ ] Battery voltage drops normally (starts 8.4V, may drop to 7.8V)
- [ ] BEC not overheating (< 60°C touchable)

### Test 12.2: Rapid Control Changes

**Goal**: Test slew rate limiting and control stability

1. Bot armed, on blocks
2. Weapon slider: Rapidly 0% → 100% → 0% (repeat 10 times)
3. Drive sticks: Rapid full deflections (10 times)
4. Self-right: Rapid press/release SH (10 times)

**Pass Criteria**:
- [ ] No resets or crashes
- [ ] Slew rate prevents instant changes (smooth acceleration)
- [ ] No motor or ESC glitches

### Test 12.3: Concurrent Load

**Goal**: Test all systems simultaneously

1. Bot armed
2. **Simultaneously**:
   - Weapon at 100%
   - Drive at full speed (all 4 motors)
   - Servo extending (SH pressed)
3. Hold for 10 seconds
4. Release all controls
5. Repeat 5 times

**Pass Criteria**:
- [ ] System remains stable under full load
- [ ] No brownouts or resets
- [ ] BEC handles current draw

### Test 12.4: Power Cycle Test (20 cycles)

**Goal**: Verify reliable startup every time

1. Power on → Verify slow blink → Power off
2. Wait 5 seconds
3. Repeat 20 times

**Pass Criteria**:
- [ ] 20/20 successful power-ons (slow blink every time)
- [ ] No stuck in error state
- [ ] No random LED patterns

**Notes**:
```
Date: ____________
Time: ____________
Result: PASS / FAIL

12.1 Continuous Operation: PASS / FAIL
  Runtime: _____ minutes
  Resets: _____ times
  Battery start: _____ V
  Battery end: _____ V
  BEC temp: Cool / Warm / Hot

12.2 Rapid Changes: PASS / FAIL
  Resets: _____ times
  Glitches observed: _____

12.3 Concurrent Load: PASS / FAIL
  Resets: _____ times
  Brownouts: YES / NO

12.4 Power Cycles: _____ / 20 successful
  Failures: _____

Overall Stress Test: PASS / FAIL
Notes: _________________________________
```

---

## Test Results Log Template

Use this template to track all test results:

```
========================================
UPVOTE BATTLEBOT - HARDWARE TEST LOG
========================================

Tester: _______________________________
Date: _________________________________
Firmware Version: Phases 1-6 Complete
Build Date: ___________________________

========================================
EQUIPMENT CHECKLIST
========================================
[ ] Arduino UNO (serial: _________)
[ ] CR8 Nano RX (bound to TX16S)
[ ] TX16S (battery: _____%)
[ ] 2S LiPo (voltage: _____V)
[ ] BEC (5V, _____A rated)
[ ] 4× Drive ESCs
[ ] 1× Weapon ESC
[ ] 1× Servo
[ ] Multimeter
[ ] Documentation printed

========================================
TEST RESULTS SUMMARY
========================================
Test 1:  Arduino Power-On           PASS / FAIL
Test 2:  Firmware Upload             PASS / FAIL
Test 3:  LED Diagnostics             PASS / FAIL
Test 4:  Receiver Power & Binding    PASS / FAIL
Test 5:  CRSF Link Verification      PASS / FAIL
Test 6:  Telemetry Validation        PASS / FAIL
Test 7:  Single Drive Motor          PASS / FAIL
Test 8:  Four-Motor Holonomic        PASS / FAIL
Test 9:  Weapon Motor Control        PASS / FAIL
Test 10: Self-Right Servo            PASS / FAIL
Test 11: Safety Interlocks           PASS / FAIL
Test 12: Stress Testing              PASS / FAIL

Overall Result: PASS / FAIL

========================================
ISSUES ENCOUNTERED
========================================
Issue #1: _____________________________
Resolution: ___________________________

Issue #2: _____________________________
Resolution: ___________________________

Issue #3: _____________________________
Resolution: ___________________________

========================================
FINAL SIGN-OFF
========================================
All tests completed: YES / NO
System ready for competition: YES / NO
Safety features verified: YES / NO

Tester Signature: _____________________
Date: _________________________________

========================================
```

---

## Next Steps After Testing

### If All Tests PASS ✅

Congratulations! Your system is validated. Next steps:

1. **Full Assembly**:
   - Install weapon blade
   - Close up armor
   - Final cable management

2. **Competition Preparation**:
   - Complete [COMPETITION_CHECKLIST.md](COMPETITION_CHECKLIST.md)
   - Practice driving
   - Test in arena (if available)

3. **Documentation Review**:
   - Read [OPERATOR_GUIDE.md](OPERATOR_GUIDE.md)
   - Familiarize with [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
   - Print [LED_STATUS_REFERENCE.md](LED_STATUS_REFERENCE.md) for quick reference

### If Tests FAIL ❌

1. **Document the failure**:
   - Which test failed?
   - What was the symptom?
   - LED pattern observed?

2. **Consult troubleshooting**:
   - See test-specific troubleshooting section
   - Check [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
   - Review [WIRING_DIAGRAM.md](WIRING_DIAGRAM.md)

3. **Systematic debugging**:
   - Isolate the problem (which component?)
   - Test component independently
   - Verify wiring with multimeter
   - Check firmware configuration

4. **Re-test after fix**:
   - Re-run failed test
   - Re-run all previous tests (regression check)

---

## Safety Reminders

**During Testing**:
- ⚠️ Never arm weapon with blade installed during bench testing
- ⚠️ Always test on blocks (wheels off ground)
- ⚠️ Keep kill switch (SD) ready at all times
- ⚠️ Disconnect battery when making wiring changes
- ⚠️ Charge LiPo in fire-safe bag
- ⚠️ Monitor BEC and ESC temperatures (should not be hot to touch)

**After Testing**:
- Always disconnect battery when done
- Store LiPo at storage voltage (7.6V) if not using within 3 days
- Document any issues or observations
- Update test log

---

## Related Documentation

- **Wiring Reference**: [WIRING_DIAGRAM.md](WIRING_DIAGRAM.md)
- **Operator Guide**: [OPERATOR_GUIDE.md](OPERATOR_GUIDE.md)
- **LED Patterns**: [LED_STATUS_REFERENCE.md](LED_STATUS_REFERENCE.md)
- **Troubleshooting**: [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
- **Competition**: [COMPETITION_CHECKLIST.md](COMPETITION_CHECKLIST.md)

---

**Document Version**: 1.0
**Firmware Version**: Phases 1-6 Complete
**Last Reviewed**: 2025-12-26

**Good luck with your build! Test thoroughly, operate safely. 🤖⚡**
