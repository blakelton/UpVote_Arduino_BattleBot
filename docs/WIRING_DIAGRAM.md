# UpVote Battlebot - Wiring Diagram

**Version**: 1.0
**Firmware**: Phases 1-6 Complete
**Last Updated**: 2025-12-26

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
| **Battery** | 2S LiPo (7.4V nominal) | 1+ | Main power |
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
| Drive ESCs (4×) | 7.4V | 100mA each | 500mA each |
| Weapon ESC | 7.4V | 100mA | 500mA |
| Servo | 5V | 100mA | 800mA (stall) |
| **Total 5V Rail** | 5V | ~300mA | **2A** |
| **Total 7.4V Rail** | 7.4V | ~500mA | **20A+** |

**BEC Requirement**: Minimum 3A at 5V (recommend 5A for headroom)

---

## Power Distribution

### Power Flow Diagram

```
┌─────────────────┐
│   2S LiPo       │
│   7.4V Nominal  │
│   8.4V Charged  │
└────────┬────────┘
         │
         ├──────────────────┐
         │                  │
    ┌────▼─────┐      ┌─────▼──────┐
    │   BEC    │      │  7.4V Rail │
    │ 7.4V→5V  │      │            │
    │  (3-5A)  │      │            │
    └────┬─────┘      └─────┬──────┘
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
- Arduino UNO (via VIN or 5V pin)
- CR8 Nano Receiver
- Self-Right Servo (via Arduino pin 9)

7.4V Rail Consumers:
- 4× Drive Motor ESCs (direct battery connection)
- 1× Weapon Motor ESC (direct battery connection)
```

### Battery Connection

**Battery Type**: 2S LiPo (2 cells in series)
- **Nominal Voltage**: 7.4V (3.7V per cell)
- **Fully Charged**: 8.4V (4.2V per cell)
- **Minimum Safe**: 6.0V (3.0V per cell)

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

### 1. Battery to BEC

**Purpose**: Convert 7.4V battery voltage to 5V for Arduino and receiver

```
Battery (+) ────┬──────────────→ BEC Input (+)
                │
                └──────────────→ 7.4V Rail (+) [to ESCs]

Battery (-) ────┴──────────────→ BEC Input (-)
                                 Ground Rail   [common ground]
```

**Wire Gauge**:
- Battery to BEC: 18-20 AWG (minimum)
- BEC to components: 22-24 AWG

**BEC Selection**:
- **Linear BEC**: Simple, may run hot at high current
- **Switching BEC**: Efficient, cooler, recommended for 3A+ loads

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

### 5. Arduino to Drive Motor ESCs (4×)

**Connection**: PWM signal from Arduino to each ESC

Each ESC has 3 wires:
- **Red** = Power (+) from battery (7.4V rail)
- **Black** = Ground (-)
- **White/Yellow** = Signal from Arduino PWM pin

```
Arduino D3 (PWM) ──────→ Front-Left ESC signal wire (white/yellow)
Arduino D5 (PWM) ──────→ Front-Right ESC signal wire (white/yellow)
Arduino D6 (PWM) ──────→ Rear-Left ESC signal wire (white/yellow)
Arduino D9 (PWM) ──────→ Rear-Right ESC signal wire (white/yellow)

Arduino GND ────────────→ All ESC black wires (ground)

Battery 7.4V+ ──────────→ All ESC red wires (power)
```

**ESC Requirements**:
- **Type**: Brushed motor ESC (bidirectional for drive motors)
- **Voltage**: Compatible with 2S LiPo (7.4V)
- **Current**: Match to motor current draw (typically 5-10A per ESC)
- **Signal**: Standard servo PWM (1000-2000 µs)

**ESC Calibration**:
- May need to calibrate ESC endpoints (1000 µs = full reverse, 1500 µs = neutral, 2000 µs = full forward)
- Refer to ESC manual for calibration procedure

**Motor Wiring**:
- ESC outputs connect to drive motors (2 wires per motor)
- Swap motor polarity to reverse direction if needed

### 6. Arduino to Weapon ESC

**Connection**: PWM signal from Arduino to weapon ESC

```
Arduino D10 (PWM) ──────→ Weapon ESC signal wire (white/yellow)
Arduino GND ────────────→ Weapon ESC black wire (ground)
Battery 7.4V+ ──────────→ Weapon ESC red wire (power)
```

**Weapon ESC Requirements**:
- **Type**: Brushed or brushless ESC (unidirectional, forward only)
- **Voltage**: Compatible with 2S LiPo (7.4V)
- **Current**: Match to weapon motor (typically 10-30A)
- **Signal**: Standard servo PWM (1000-2000 µs)
  - 1000 µs = 0% throttle (stopped)
  - 2000 µs = 100% throttle (full speed)

**Weapon Motor Wiring**:
- Brushed motor: 2 wires (polarity determines spin direction)
- Brushless motor: 3 wires (A, B, C phases)

**⚠️ SAFETY**:
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
- ☐ **BEC rated for 3A+ at 5V** (recommend 5A)
- ☐ **ESCs rated for motor current** (check motor specs)
- ☐ **Servo compatible with 5V** (standard hobby servo)
- ☐ **Wire gauge sufficient**:
  - 18-20 AWG for battery mains
  - 22-24 AWG for signal wires
- ☐ **Heat shrink tubing** for exposed connections
- ☐ **Zip ties** for cable management

### Assembly Checklist

**Power Connections**:
- ☐ Battery (+) to BEC input (+)
- ☐ Battery (+) to 7.4V rail (ESC power)
- ☐ Battery (-) to ground rail (common)
- ☐ BEC 5V output to Arduino 5V pin
- ☐ BEC GND to Arduino GND
- ☐ BEC 5V output to CR8 Nano power pins
- ☐ All grounds connected to common ground rail

**Signal Connections**:
- ☐ CR8 TX to Arduino D0 (RX)
- ☐ CR8 RX to Arduino D1 (TX) [if telemetry enabled]
- ☐ Arduino D3 to Front-Left ESC signal
- ☐ Arduino D5 to Front-Right ESC signal
- ☐ Arduino D6 to Rear-Left ESC signal
- ☐ Arduino D9 to Rear-Right ESC signal
- ☐ Arduino D10 to Weapon ESC signal
- ☐ Arduino D11 to Servo signal

**Motor Connections**:
- ☐ Front-Left ESC to FL motor
- ☐ Front-Right ESC to FR motor
- ☐ Rear-Left ESC to RL motor
- ☐ Rear-Right ESC to RR motor
- ☐ Weapon ESC to weapon motor

**Verification**:
- ☐ **Polarity check**: Multimeter on battery connector (Red=+, Black=-)
- ☐ **Voltage check**: Measure battery voltage (should be 7.4-8.4V)
- ☐ **Continuity check**: Verify all grounds connected
- ☐ **Isolation check**: Verify no shorts between + and - rails
- ☐ **Visual inspection**: No exposed wire, no pinched cables

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
1. ☐ Battery charged? (Measure voltage: should be > 7.0V)
2. ☐ Battery connector secure?
3. ☐ BEC receiving power? (Measure BEC input voltage)
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
1. ☐ ESCs receiving power? (7.4V from battery)
2. ☐ ESC signal wires connected to correct Arduino pins?
3. ☐ ESC ground wires connected to Arduino GND?
4. ☐ ESCs calibrated? (Some ESCs require endpoint calibration)
5. ☐ Motors connected to ESC outputs?

**Fixes**:
- Check ESC power connections
- Verify pin assignments (see Pin Assignment Table)
- Calibrate ESCs if required (refer to ESC manual)
- Test motors directly with ESC tester

### Weapon Not Spinning

**Symptoms**:
- Bot armed (fast blink LED)
- Weapon slider moved, but weapon doesn't spin

**Checks**:
1. ☐ Bot armed? (LED fast blink, not slow blink?)
2. ☐ Weapon slider > 10%? (Must overcome arming hysteresis)
3. ☐ Weapon ESC receiving power? (7.4V from battery)
4. ☐ Weapon ESC signal wire connected to Arduino D10?
5. ☐ Weapon ESC ground connected to Arduino GND?
6. ☐ Weapon motor connected to ESC output?

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
2. ☐ Battery voltage sufficient? (> 7.0V under load)
3. ☐ BEC not overheating?
4. ☐ No short circuits?
5. ☐ Capacitors on ESCs? (Some ESCs lack filtering)

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
Max input: 8.4V × 0.18 = 1.5V (safe for Arduino 5V ADC)
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

**Document Version**: 1.0
**Firmware Version**: Phases 1-6 Complete
**Last Reviewed**: 2025-12-26

**Questions?** See [TROUBLESHOOTING.md](TROUBLESHOOTING.md) for detailed diagnostics
