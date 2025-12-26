# Component Registry

**Generated**: 2025-12-26
**Total Components**: 9
**Total Files**: 18 (9 headers + 9 implementations)
**Total LOC**: ~1,993

## Components by Domain

### Embedded Components

All components are embedded C/C++ for Arduino UNO (ATmega328P).

#### Core Components

1. **Config** - `include/config.h`
   - Type: Configuration header
   - Lines: ~197
   - Purpose: Central compile-time constants and pin definitions

2. **State** - `include/state.h`, `src/state.cpp`
   - Type: Data structure module
   - Lines: ~159
   - Purpose: Global runtime state structure and initialization

3. **Main** - `src/main.cpp`
   - Type: Entry point
   - Lines: ~123
   - Purpose: Control loop orchestration and timing

#### Safety & Monitoring Components

4. **Safety** - `include/safety.h`, `src/safety.cpp`
   - Type: Safety system
   - Lines: ~112
   - Purpose: Watchdog management, error handling, arming state
   - Critical: Yes (safety-critical)

5. **Diagnostics** - `include/diagnostics.h`, `src/diagnostics.cpp`
   - Type: Monitoring & diagnostics
   - Lines: ~172
   - Purpose: LED status indication, RAM monitoring, error code display

#### Input/Output Components

6. **Input** - `include/input.h`, `src/input.cpp`
   - Type: Receiver interface
   - Lines: ~489
   - Purpose: CRSF protocol parser, telemetry, channel decoding
   - Complexity: HIGH (bit manipulation, state machine)

7. **Actuators** - `include/actuators.h`, `src/actuators.cpp`
   - Type: Hardware abstraction
   - Lines: ~243
   - Purpose: Motor control, ESC/servo output, shift register management

#### Control Logic Components

8. **Mixing** - `include/mixing.h`, `src/mixing.cpp`
   - Type: Drive control
   - Lines: ~195
   - Purpose: Holonomic drive mixing, expo curves, normalization

9. **Weapon** - `include/weapon.h`, `src/weapon.cpp`
   - Type: Weapon control
   - Lines: ~211
   - Purpose: Arming state machine, weapon output scaling
   - Critical: Yes (safety-critical)

10. **Servo** - `include/servo.h`, `src/servo.cpp`
    - Type: Servo control
    - Lines: ~101
    - Purpose: Self-righting servo with rate limiting

## Component Dependencies

```
main.cpp
  ├── config.h (constants)
  ├── state.h (global state)
  ├── safety.h → safety.cpp
  ├── diagnostics.h → diagnostics.cpp
  ├── input.h → input.cpp
  ├── actuators.h → actuators.cpp
  ├── mixing.h → mixing.cpp
  ├── weapon.h → weapon.cpp
  └── servo.h → servo.cpp

All modules depend on:
  - config.h (constants)
  - state.h (global state)
  - Arduino.h (framework)
```

## Complexity Hotspots

| Component | Est. CC | Reason |
|-----------|---------|--------|
| Input | HIGH | CRSF state machine, bit unpacking, CRC calculation |
| Weapon | MEDIUM | Arming state machine with multiple conditions |
| Mixing | MEDIUM | Holonomic math, normalization |
| Actuators | MEDIUM | Motor control with slew rate limiting |
| Diagnostics | MEDIUM | Error code state machine |
| Safety | LOW | Simple validation logic |
| Servo | LOW | Simple rate limiting |

## Memory Profile

### Static Memory Footprint (Estimated)

| Component | RAM Usage | Notes |
|-----------|-----------|-------|
| State | ~180 bytes | RuntimeState structure |
| Input | ~320 bytes | CRC table in PROGMEM, parser state ~168B |
| Actuators | ~10 bytes | Previous motor values, shift register state |
| Mixing | ~8 bytes | Mode params cache |
| Weapon | ~2 bytes | Previous weapon value |
| Servo | ~2 bytes | Previous servo value |
| Diagnostics | minimal | Uses state structure |
| **Total** | **~522 bytes** | Static allocations |

### Flash Memory Footprint (Estimated)

| Component | Flash Usage | Notes |
|-----------|-------------|-------|
| Input | HIGH | CRC table (256B), complex parsing logic |
| Mixing | MEDIUM | Float math, holonomic formulas |
| Weapon | MEDIUM | Complex state machine |
| Actuators | MEDIUM | Motor control logic |
| Others | LOW | Simple logic |

## Safety-Critical Components

1. **Safety** - Core safety system
2. **Weapon** - Arming state machine (weapon must NEVER activate unintentionally)
3. **Input** - Link health monitoring (failsafe on timeout)
4. **Actuators** - Emergency stop capability

## Analysis Priority

1. **Input** (highest complexity, protocol parsing)
2. **Weapon** (safety-critical arming logic)
3. **Safety** (foundational safety system)
4. **Actuators** (hardware interface layer)
5. **Mixing** (control algorithm)
6. **Diagnostics** (error handling)
7. **Servo** (simple control)
8. **State** (data structure)
9. **Config** (constants only)
