# UpVote Battlebot - Wiring Diagram

**Version**: 1.1 (3S Upgrade)
**Firmware**: Phases 1-6 Complete
**Last Updated**: 2025-12-29

---

## Table of Contents

1. [System Overview](#system-overview)
2. [Power Distribution](#power-distribution)
3. [Arduino UNO Connections](#arduino-uno-connections)
4. [Pin Assignment Table](#pin-assignment-table)
5. [Component Wiring Details](#component-wiring-details)
6. [Wiring Checklist](#wiring-checklist)
7. [Common Wiring Mistakes](#common-wiring-mistakes)
8. [Troubleshooting](#troubleshooting)

---

## System Overview

### Component List

| Component | Model/Type | Quantity | Purpose |
|-----------|------------|----------|---------|
| **Microcontroller** | Arduino UNO (ATmega328P) | 1 | Main controller |
| **Receiver** | CR8 Nano ExpressLRS | 1 | 2.4GHz RC receiver |
| **Transmitter** | RadioMaster TX16S | 1 | Remote control |
| **Battery** | 3S LiPo (11.1V nominal) | 1+ | Main power |
| **BEC** | 5V BEC (3A minimum) | 1 | 5V power for Arduino/RX |
| **Drive Motors** | Brushed DC motors | 4 | Holonomic drive |
| **Drive ESCs** | Brushed ESC (bidirectional) | 4 | Motor control |
| **Weapon Motor** | Brushed DC motor | 1 | Weapon spinner |
| **Weapon ESC** | Brushed ESC (unidirectional) | 1 | Weapon control |
| **Self-Right Servo** | Standard hobby servo | 1 | Self-righting mechanism |

### Power Requirements

| Component | Voltage | Current (typical) | Current (max) |
|-----------|---------|-------------------|---------------|
| Arduino UNO | 5V | 50mA | 200mA |
| CR8 Nano RX | 5V | 50mA | 100mA |
| Drive ESCs (4×) | 11.1V | 100mA each | 500mA each |
| Weapon ESC | 11.1V | 100mA | 500mA |
| Servo | 5V | 100mA | 800mA (stall) |
| **Total 5V Rail** | 5V | ~300mA | **2A** |
| **Total 11.1V Rail** | 11.1V | ~500mA | **20A+** |

**BEC Requirement**: Minimum 3A at 5V (recommend 5A for headroom)

---

## Power Distribution

### Power Flow Diagram

```
┌─────────────────┐
│   3S LiPo       │
│  11.1V Nominal  │
│  12.6V Charged  │
└────────┬────────┘
         │
         ├──────────────────┐
         │                  │
    ┌────▼─────┐      ┌─────▼───────┐
    │   BEC    │      │  11.1V Rail │
    │ 11.1V→5V │      │             │
    │  (3-5A)  │      │             │
    └────┬─────┘      └─────┬───────┘
         │                  │
    ┌────▼─────┐           │
    │ 5V Rail  │           │
    │          │           │
    └────┬─────┘           │
         │                 │
    ┌────▼─────────────────▼─────┐
    │     Ground Common Rail     │
    │  (All grounds connected)   │
    └────────────────────────────┘

5V Rail Consumers:
- Arduino UNO (via 5V pin)
- CR8 Nano Receiver
- Self-Right Servo (via Arduino pin 11)

11.1V Rail Consumers:
- L293D Motor Shield (motor power input)
- Weapon Motor ESC (direct battery connection)
```

### Battery Connection

**Battery Type**: 3S LiPo (3 cells in series)
- **Nominal Voltage**: 11.1V (3.7V per cell)
- **Fully Charged**: 12.6V (4.2V per cell)
- **Minimum Safe**: 9.0V (3.0V per cell)

**Connector**: XT60, XT30, or Deans (match to battery)

**Polarity**:
- **Red wire** = Positive (+)
- **Black wire** = Negative (-)

**⚠️ CRITICAL**: Reverse polarity will destroy electronics!

---

## Arduino UNO Connections

### Arduino Pinout Overview

```
                    ┌─────────────────┐
                    │   ARDUINO UNO   │
                    │   ATmega328P    │
                    ├─────────────────┤
                    │                 │
         RESET ─────┤ RESET       5V  ├──── 5V Rail (from BEC)
                    │                 │
                    │            GND  ├──── Ground Rail
                    │                 │
       CR8 TX ──────┤ D0 (RX)    VIN  ├──── (not used)
                    │                 │
                    │            A0   ├──── (unused)
                    │                 │
    Drive FL ───────┤ D3  (PWM)  A1   ├──── (unused)
                    │                 │
    Drive FR ───────┤ D5  (PWM)  A2   ├──── (unused)
                    │                 │
    Drive RL ───────┤ D6  (PWM)  A3   ├──── (unused)
                    │                 │
    Drive RR ───────┤ D9  (PWM)  A4   ├──── (unused)
                    │                 │
    Weapon ─────────┤ D10 (PWM)  A5   ├──── (unused)
                    │                 │
    Servo ──────────┤ D11 (PWM)       │
                    │                 │
                    │ D13 (LED)       │ ← Built-in status LED
                    │                 │
                    └─────────────────┘
```

### Pin Functions

| Pin | Type | Function | Connected To | Signal |
|-----|------|----------|--------------|--------|
| **D0 (RX)** | UART | CRSF Receiver Input | CR8 Nano TX pin | 420kbaud serial |
| **D1 (TX)** | UART | CRSF Telemetry Output | CR8 Nano RX pin | 420kbaud serial (optional) |
| **D3** | PWM | Front-Left Drive Motor | FL ESC signal wire | PWM (490Hz) |
| **D5** | PWM | Front-Right Drive Motor | FR ESC signal wire | PWM (490Hz) |
| **D6** | PWM | Rear-Left Drive Motor | RL ESC signal wire | PWM (490Hz) |
| **D9** | PWM | Rear-Right Drive Motor | RR ESC signal wire | PWM (490Hz) |
| **D10** | PWM | Weapon Motor | Weapon ESC signal wire | PWM (490Hz) |
| **D11** | PWM | Self-Right Servo | Servo signal wire | PWM (490Hz) |
| **D13** | Output | Status LED | Built-in LED | High/Low |
| **5V** | Power | 5V Supply | BEC 5V output | 5V DC |
| **GND** | Ground | Ground Reference | Ground rail | 0V |

**Note**: Analog pins A0-A5 are currently unused and available for future expansion (battery monitoring, sensors, etc.)

---

## Pin Assignment Table

### Complete Pin Map

```cpp
// Pin definitions from include/config.h

// Phase 2: CRSF Communication
#define PIN_CRSF_RX         0  // Hardware Serial RX (CR8 Nano TX)
#define PIN_CRSF_TX         1  // Hardware Serial TX (CR8 Nano RX) - telemetry

// Phase 4: Drive Motor Outputs
#define PIN_MOTOR_FL        3  // Front-Left motor ESC (PWM)
#define PIN_MOTOR_FR        5  // Front-Right motor ESC (PWM)
#define PIN_MOTOR_RL        6  // Rear-Left motor ESC (PWM)
#define PIN_MOTOR_RR        9  // Rear-Right motor ESC (PWM)

// Phase 5: Weapon Motor Output
#define PIN_WEAPON_ESC     10  // Weapon motor ESC (PWM)

// Phase 6: Self-Right Servo
#define PIN_SELFRIGHT_SERVO 11 // Self-right servo (PWM)

// Phase 1: Status LED
#define PIN_STATUS_LED     13  // Built-in LED (on Arduino UNO)
```

---

## Component Wiring Details

### 1. Battery to BEC and Power Distribution

**Purpose**: Convert 11.1V battery voltage to 5V for Arduino and receiver; distribute 11.1V to motor drivers

```
Battery (+) ────┬──────────────→ BEC Input (+)
                │
                ├──────────────→ L293D Shield Motor Power (+)
                │
                └──────────────→ Weapon ESC Power (+)

Battery (-) ────┴──────────────→ BEC Input (-)
                                 Ground Rail   [common ground]
```

**Wire Gauge**:
- Battery to BEC: 18-20 AWG (minimum)
- BEC to components: 22-24 AWG

**BEC Selection**:
- **Switching BEC**: Required for 3S operation (efficient, minimal heat)
- **Example**: MEIVIFPV Micro BEC (2S-6S input, 5V/3A output) ✅
- **Linear BEC**: NOT recommended for 3S (excessive heat dissipation)

### 2. BEC to Arduino UNO

**Connection**: BEC 5V output → Arduino 5V pin

```
BEC 5V (+) ──────→ Arduino 5V pin
BEC GND    ──────→ Arduino GND pin
```

**⚠️ WARNING**:
- Do NOT connect BEC to VIN pin (VIN expects 7-12V)
- Do NOT use USB power simultaneously with BEC (can damage USB port)
- Arduino can draw up to 200mA; ensure BEC rated for total 5V load

**Alternative**: Connect BEC to Arduino VIN if BEC outputs 7-9V (uncommon for 5V BECs)

### 3. BEC to CR8 Nano Receiver

**Connection**: BEC 5V output → CR8 Nano via any servo connector

```
BEC 5V (+) ──────→ CR8 Red wire (any channel pin)
BEC GND    ──────→ CR8 Black wire (any channel pin)
```

**Recommended Channel**: Use CH1 or CH2 power pins
**Wire Colors** (standard servo cable):
- **Red** = 5V
- **Black/Brown** = Ground
- **White/Yellow/Orange** = Signal (not used for power)

**Current Draw**: CR8 Nano draws ~50-100mA typical

### 4. CR8 Nano to Arduino (CRSF Serial)

**Connection**: UART communication at 420kbaud

```
CR8 Nano TX pin ──────→ Arduino D0 (RX)
CR8 Nano RX pin ──────→ Arduino D1 (TX) [optional, for telemetry]
CR8 GND         ──────→ Arduino GND [already connected via power]
```

**CR8 Nano Pinout**:
- **TX Pin**: Usually labeled "TX" or "SBUS/CRSF"
- **RX Pin**: Usually labeled "RX"

**⚠️ CRITICAL**:
- CR8 TX → Arduino RX (crossover connection)
- CR8 RX → Arduino TX (crossover connection)
- Both are 5V logic (direct connection safe)
- If telemetry not needed, only connect CR8 TX to Arduino RX

**CRSF Protocol**: 420000 baud, 8N1 (8 data bits, no parity, 1 stop bit)

### 5. L293D Motor Shield to Drive Motors (4×)

**NOTE**: This system uses an L293D Motor Driver Shield, NOT individual ESCs for drive motors.

**L293D Shield Connection**:
- **Motor Power Input**: Battery 11.1V+ → Shield external power terminal
- **Logic Power**: Arduino 5V → Shield (via stacking headers)
- **PWM Control**: Arduino pins D3, D5, D6, D9 → Shield motor enable pins
- **Direction Control**: 74HC595 shift register (pins D4, D7, D8, D12)

**L293D Shield Specifications**:
- **Motor voltage range**: 4.5V - 36V (3S @11.1V is perfect ✅)
- **Current per channel**: 600mA continuous (1.2A peak)
- **Channels**: 4× bidirectional DC motor outputs
- **Control**: PWM speed + shift register direction

**Motor Outputs**:
- **M1**: Rear-Left drive motor (controlled by D5 PWM + shift register)
- **M2**: Rear-Right drive motor (controlled by D6 PWM + shift register)
- **M3**: Front-Left drive motor (controlled by D9 PWM + shift register)
- **M4**: Front-Right drive motor (controlled by D3 PWM + shift register)

**⚠️ 3S THERMAL CONSIDERATION**:
- L293D chip may run warmer on 3S due to higher voltage drop
- Monitor chip temperature during initial testing (<80°C acceptable)
- Consider adding heatsink if chip exceeds 70°C

**TT Motor Specifications** (typical):
- **Voltage rating**: 3-6V (nominal)
- **3S operation**: Motors receive PWM-averaged voltage from L293D
- **Firmware protection**: 80% max duty cycle limit (config.h: MOTOR_DUTY_CLAMP_MAX = 204)
- **Effective voltage**: ~8.9V max (80% of 11.1V) at full throttle

**Motor Wiring**:
- L293D outputs connect to drive motors (2 wires per motor)
- Swap motor polarity to reverse direction if needed

### 6. Arduino to Weapon ESC

**Connection**: PWM signal from Arduino to weapon ESC

```
Arduino D10 (PWM) ──────→ Weapon ESC signal wire (white/yellow)
Arduino GND ────────────→ Weapon ESC black wire (ground)
Battery 11.1V+ ─────────→ Weapon ESC red wire (power)
```

**Weapon ESC Requirements**:
- **Type**: Brushless ESC (unidirectional, forward only)
- **Model**: HW30A or similar (2-4S rated) ✅
- **Voltage**: 3S LiPo (11.1V nominal, 12.6V charged) - within 2-4S range
- **Current**: 30A continuous (match to weapon motor)
- **Signal**: Standard servo PWM (1000-2000 µs)
  - 1000 µs = 0% throttle (stopped)
  - 2000 µs = 100% throttle (full speed)

**Weapon Motor Specifications**:
- **Model**: RS2205 brushless motor (2-4S rated) ✅
- **3S performance**: ~50% faster spin speed vs 2S
- **Kinetic energy**: ~2.25× higher than 2S operation ⚠️
- **KV rating**: Verify motor KV (lower KV preferred for 3S to reduce heat)

**⚠️ 3S WEAPON SAFETY WARNINGS**:
- Weapon spins **significantly faster** on 3S (50% more voltage)
- **MUST re-balance weapon** before first 3S test (critical at high RPM)
- Test gradually: 20% → 40% → 60% → 80% throttle
- Monitor motor temperature (<80°C acceptable)
- Weapon ESC should only spin in one direction (no reverse)
- Ensure weapon spins in correct direction before installing blade

### 7. Arduino to Self-Right Servo

**Connection**: PWM signal from Arduino to servo

```
Arduino D11 (PWM) ──────→ Servo signal wire (white/yellow/orange)
Arduino 5V ─────────────→ Servo red wire (power)
Arduino GND ────────────→ Servo black/brown wire (ground)
```

**Servo Power**:
- **Option 1**: Power servo from Arduino 5V pin
  - **Pros**: Simple wiring
  - **Cons**: Arduino 5V pin limited to ~500mA; servo stall current can exceed this
- **Option 2**: Power servo from separate 5V BEC
  - **Pros**: Dedicated power, no Arduino overload risk
  - **Cons**: Additional BEC required

**Recommended**: Use Arduino 5V pin if servo is small (< 500mA stall). Use separate BEC if servo draws > 500mA.

**Servo Requirements**:
- **Type**: Standard hobby servo (analog or digital)
- **Voltage**: 5V
- **Torque**: Depends on self-right mechanism design
- **Signal**: Standard servo PWM
  - Neutral: 1500 µs
  - Retract: 700 µs (configurable)
  - Extend: 2300 µs (configurable)

**Firmware Configuration** (in `include/config.h`):
```cpp
#define SERVO_ENDPOINT_RETRACT  700   // Retracted position (µs)
#define SERVO_ENDPOINT_EXTEND  2300   // Extended position (µs)
#define SERVO_NEUTRAL_US       1500   // Neutral position (µs)
```

---

## Wiring Checklist

### Pre-Assembly Checklist

- ☐ **Battery connector matches LiPo** (XT60, XT30, Deans)
- ☐ **BEC rated for 2S-6S input, 5V/3A+ output** (switching type required)
- ☐ **L293D shield rated for 3S** (4.5-36V range ✅)
- ☐ **Weapon ESC rated for 3S** (2-4S range, e.g., HW30A ✅)
- ☐ **Weapon motor rated for 3S** (2-4S range, e.g., RS2205 ✅)
- ☐ **Servo compatible with 5V** (standard hobby servo)
- ☐ **Wire gauge sufficient**:
  - 18-20 AWG for battery mains (adequate for 3S)
  - 22-24 AWG for signal wires
- ☐ **Heat shrink tubing** for exposed connections
- ☐ **Zip ties** for cable management

### Assembly Checklist

**Power Connections**:
- ☐ Battery (+) to BEC input (+)
- ☐ Battery (+) to L293D shield motor power terminal
- ☐ Battery (+) to Weapon ESC power (+)
- ☐ Battery (-) to ground rail (common)
- ☐ BEC 5V output to Arduino 5V pin
- ☐ BEC GND to Arduino GND
- ☐ BEC 5V output to CR8 Nano power pins
- ☐ All grounds connected to common ground rail

**Signal Connections**:
- ☐ CR8 TX to Arduino D0 (RX)
- ☐ CR8 RX to Arduino D1 (TX) [if telemetry enabled]
- ☐ L293D shield stacked on Arduino (PWM pins: D3, D5, D6, D9)
- ☐ Shift register control pins: D4, D7, D8, D12
- ☐ Arduino D10 to Weapon ESC signal
- ☐ Arduino D11 to Servo signal

**Motor Connections**:
- ☐ L293D M1 to Rear-Left motor
- ☐ L293D M2 to Rear-Right motor
- ☐ L293D M3 to Front-Left motor
- ☐ L293D M4 to Front-Right motor (Note: check pin mapping - may vary)
- ☐ Weapon ESC to weapon motor (RS2205 brushless)

**Verification**:
- ☐ **Polarity check**: Multimeter on battery connector (Red=+, Black=-)
- ☐ **Voltage check**: Measure battery voltage (should be 11.1-12.6V for 3S)
- ☐ **Continuity check**: Verify all grounds connected
- ☐ **Isolation check**: Verify no shorts between + and - rails
- ☐ **Visual inspection**: No exposed wire, no pinched cables
- ☐ **L293D thermal**: Monitor chip temperature during initial 3S testing

---

## Common Wiring Mistakes

### Mistake 1: Reversed Polarity

**Symptom**: Components smoke or fail to power on
**Cause**: Battery connected backward (+ and - swapped)
**Prevention**:
- Use keyed connectors (XT60/XT30 prevent reverse connection)
- Always check polarity with multimeter before connecting
- Mark battery connector with red (+) and black (-) labels

**⚠️ CRITICAL**: Reverse polarity can destroy Arduino, BEC, and receiver instantly!

### Mistake 2: BEC Connected to Arduino VIN

**Symptom**: BEC overheats or Arduino reboots randomly
**Cause**: 5V BEC connected to VIN pin (expects 7-12V)
**Fix**:
- Connect BEC 5V output to Arduino **5V pin** (not VIN)
- VIN pin has onboard regulator; feeding it 5V bypasses regulator and can cause instability

### Mistake 3: Missing Common Ground

**Symptom**: Erratic behavior, signal noise, random resets
**Cause**: Arduino ground not connected to battery ground
**Fix**:
- Ensure Arduino GND, BEC GND, ESC grounds, and battery (-) all connected to common ground rail
- Ground is the reference voltage; without common ground, signals are meaningless

### Mistake 4: Insufficient BEC Current Rating

**Symptom**: BEC overheats, voltage sags, Arduino resets
**Cause**: BEC rated for < 3A; total 5V load exceeds capacity
**Fix**:
- Use 3A minimum BEC (recommend 5A for headroom)
- Calculate total 5V load: Arduino (200mA) + RX (100mA) + Servo (800mA stall) = **1.1A typical, 2A peak**

### Mistake 5: TX/RX Not Crossed

**Symptom**: No CRSF communication, LED shows link loss (solid ON)
**Cause**: CR8 TX connected to Arduino TX (should be RX)
**Fix**:
- **Crossover connection**: CR8 TX → Arduino RX (D0)
- **Crossover connection**: CR8 RX → Arduino TX (D1)

### Mistake 6: ESC Signal Ground Not Connected

**Symptom**: ESC does not respond to PWM signal
**Cause**: ESC signal ground (black wire) not connected to Arduino GND
**Fix**:
- Connect all ESC black wires to Arduino GND
- ESC needs ground reference to read PWM signal

### Mistake 7: Servo Overloading Arduino 5V Pin

**Symptom**: Arduino resets when servo moves, brownouts
**Cause**: Servo stall current > 500mA (Arduino 5V pin limit)
**Fix**:
- Use separate 5V BEC for servo
- Or use smaller servo (< 500mA stall)

---

## Troubleshooting

### No Power to Arduino

**Symptoms**:
- Arduino power LED not lit
- No status LED blink

**Checks**:
1. ☐ Battery charged? (Measure voltage: should be > 10.5V for 3S)
2. ☐ Battery connector secure?
3. ☐ BEC receiving power? (Measure BEC input voltage: should be 11.1-12.6V)
4. ☐ BEC outputting 5V? (Measure BEC output with multimeter)
5. ☐ BEC 5V connected to Arduino 5V pin (not VIN)?
6. ☐ Ground connected?

**Fixes**:
- Charge battery
- Secure connections
- Replace BEC if faulty

### LED Shows Link Loss (Solid ON)

**Symptoms**:
- Arduino powers on
- LED solid ON (should be slow blink)

**Checks**:
1. ☐ TX16S transmitter powered on?
2. ☐ Correct model selected on TX16S?
3. ☐ CR8 Nano receiver powered? (Green LED blinking on RX?)
4. ☐ CR8 TX connected to Arduino D0 (RX)?
5. ☐ CR8 RX connected to Arduino D1 (TX)? [if telemetry enabled]
6. ☐ UART baudrate correct? (Should be 420000 in firmware)

**Fixes**:
- Power on transmitter
- Select correct model
- Check CRSF wiring (CR8 TX → Arduino RX)
- Rebind receiver if needed

### Motors Not Responding

**Symptoms**:
- Link OK (slow/fast blink LED)
- Sticks moved, but motors don't spin

**Checks**:
1. ☐ L293D shield receiving motor power? (11.1V from battery to external terminal)
2. ☐ L293D shield stacked correctly on Arduino?
3. ☐ PWM pins (D3, D5, D6, D9) making good contact?
4. ☐ Shift register pins (D4, D7, D8, D12) connected?
5. ☐ Motors connected to L293D outputs (M1-M4)?

**Fixes**:
- Check L293D shield motor power terminal connection
- Verify shield is seated properly on Arduino headers
- Test motors directly (connect motor to battery via resistor to verify motor works)
- Check L293D chip temperature (may be overheating on 3S)

### Weapon Not Spinning

**Symptoms**:
- Bot armed (fast blink LED)
- Weapon slider moved, but weapon doesn't spin

**Checks**:
1. ☐ Bot armed? (LED fast blink, not slow blink?)
2. ☐ Weapon slider > 10%? (Must overcome arming hysteresis)
3. ☐ Weapon ESC receiving power? (11.1V from battery)
4. ☐ Weapon ESC signal wire connected to Arduino D10?
5. ☐ Weapon ESC ground connected to Arduino GND?
6. ☐ Weapon motor connected to ESC output?
7. ☐ Weapon ESC calibrated for 3S? (Some ESCs require recalibration)

**Fixes**:
- Arm bot (SA switch UP, slider at 0%)
- Move slider to > 10%
- Check weapon ESC wiring
- Test weapon motor directly

### Servo Not Moving

**Symptoms**:
- Self-right button pressed (SH on TX16S)
- Servo doesn't move

**Checks**:
1. ☐ Servo receiving 5V power?
2. ☐ Servo signal wire connected to Arduino D11?
3. ☐ Servo ground connected to Arduino GND?
4. ☐ Kill switch inactive? (SD switch DOWN)
5. ☐ Link OK?

**Fixes**:
- Check servo power (should be 5V)
- Verify signal connection to D11
- Ensure SD switch DOWN (kill switch inactive)
- Test servo with servo tester

### Erratic Behavior / Random Resets

**Symptoms**:
- Arduino resets randomly
- Motors twitch or jitter
- LED pattern inconsistent

**Checks**:
1. ☐ All grounds connected to common rail?
2. ☐ Battery voltage sufficient? (> 10.5V under load for 3S)
3. ☐ BEC not overheating? (switching BEC should stay cool)
4. ☐ L293D chip not overheating? (<80°C acceptable)
5. ☐ No short circuits?
6. ☐ Capacitors on power rails? (1000µF recommended)

**Fixes**:
- Connect all grounds together
- Charge battery
- Use higher-rated BEC
- Add capacitor (1000µF, 16V) across battery terminals (reduces voltage spikes)

---

## Advanced: Optional Connections

### Battery Voltage Monitoring (Future Enhancement)

**Purpose**: Monitor battery voltage and send to transmitter via telemetry

**Hardware**:
- Voltage divider resistors (10kΩ and 2.2kΩ)
- Connect to Arduino analog pin (A0-A5)

**Connection**:
```
Battery (+) ────┬──── 10kΩ resistor ────┬──── Arduino A0
                                        │
                                     2.2kΩ resistor
                                        │
Battery (-) ────┴───────────────────────┴──── Arduino GND

Voltage divider ratio: 2.2kΩ / (10kΩ + 2.2kΩ) = 0.18
Max input (3S charged): 12.6V × 0.18 = 2.27V (safe for Arduino 5V ADC ✅)
Min input (3S low): 9.0V × 0.18 = 1.62V
Scale factor for firmware: 0.0271 V/ADC_count
```

**Firmware Change**: Uncomment `#define BATTERY_MONITOR_PIN A0` in `include/config.h`

### External Kill Switch (Physical)

**Purpose**: Manual kill switch accessible outside arena (required by some events)

**Hardware**:
- SPST switch or removable link
- 22 AWG wire

**Connection**:
```
Battery (+) ────┬──── Switch ────┬──── Main power distribution
                │                 │
                └─────────────────┘ [Switch open = power cut]
```

**Note**: This is a physical power cut, independent of firmware kill switch (SD on TX16S)

---

## Wire Color Standards

### Standard Servo Wire Colors

| Color | Function |
|-------|----------|
| **Red** | Power (+5V) |
| **Black or Brown** | Ground (0V) |
| **White, Yellow, or Orange** | Signal (PWM) |

### Standard Power Wire Colors

| Color | Function |
|-------|----------|
| **Red** | Positive (+) |
| **Black** | Negative (-) Ground |

### CRSF/Serial Wire Colors (CR8 Nano)

May vary by manufacturer; refer to receiver documentation:
- **TX** (transmit): Sends data to Arduino
- **RX** (receive): Receives data from Arduino
- **5V**: Power
- **GND**: Ground

---

## Related Documentation

- **Operator Guide**: [OPERATOR_GUIDE.md](OPERATOR_GUIDE.md)
- **Troubleshooting**: [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
- **Competition Checklist**: [COMPETITION_CHECKLIST.md](COMPETITION_CHECKLIST.md)
- **LED Patterns**: [LED_STATUS_REFERENCE.md](LED_STATUS_REFERENCE.md)

---

## Safety Warnings

**⚠️ ELECTRICAL HAZARDS**:
- Always disconnect battery before making wiring changes
- Use properly rated wire gauge for current levels
- Insulate all exposed connections with heat shrink
- Never short battery terminals (can cause fire)
- LiPo batteries can catch fire if damaged; handle with care

**⚠️ MECHANICAL HAZARDS**:
- Weapon can spin at high speed; keep clear when powered
- Test weapon balance before installing in bot
- Never arm weapon outside arena

**⚠️ TESTING BEST PRACTICES**:
- Perform initial power-on test with weapon motor disconnected
- Place bot on blocks (wheels off ground) for drive testing
- Keep weapon slider at 0% until ready to test weapon
- Have kill switch (SD on TX16S) ready at all times

---

**Document Version**: 1.1 (3S Upgrade)
**Firmware Version**: Phases 1-6 Complete (3S Compatible)
**Last Reviewed**: 2025-12-29

**⚠️ 3S UPGRADE NOTES**:
- System upgraded from 2S (7.4V) to 3S (11.1V) operation
- L293D shield handles 3S voltage (4.5-36V range)
- Weapon motor spins ~50% faster (2.25× kinetic energy)
- Monitor L293D and TT motor temperatures during initial testing
- See [3S_UPGRADE_GUIDE.md](3S_UPGRADE_GUIDE.md) for detailed upgrade information

**Questions?** See [TROUBLESHOOTING.md](TROUBLESHOOTING.md) for detailed diagnostics
