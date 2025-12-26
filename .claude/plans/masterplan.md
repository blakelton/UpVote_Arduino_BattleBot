# Master Plan: UpVote Battlebot Firmware

> Safety-critical holonomic drive battlebot firmware for Arduino UNO
> Last updated: 2025-12-25

## Executive Summary

UpVote is a competitive battlebot featuring holonomic drive capabilities, powered by an Arduino UNO with safety-critical weapon control. This firmware implements a complete control system for a 4-motor holonomic drive platform with an independently controlled brushless weapon motor and self-righting mechanism. The system prioritizes safety above all else, with multiple layers of failsafe protection to prevent unintended weapon activation or loss of control.

The development follows a 7-phase incremental approach, validating each subsystem independently before integration. Each phase builds upon proven, tested components, minimizing risk and ensuring the robot is safe at every stage of development.

## Vision Statement

Create a robust, competition-ready battlebot control system that is **safe by design**, providing predictable holonomic control with bulletproof weapon safety, enabling confident operation in high-stress combat scenarios while gracefully handling all failure modes.

## Problem Statement

Combat robots require simultaneous control of drive systems and weapons in unpredictable, high-stress environments. The challenges include:

- **Safety**: Weapons can cause serious injury if they activate unexpectedly due to software bugs, radio link loss, or power glitches
- **Control complexity**: Holonomic drive requires real-time mixing of three independent motion axes
- **Hardware limitations**: Arduino UNO has only 2KB RAM and limited processing power
- **Reliability**: Must operate reliably despite electrical noise, voltage sags, and mechanical impacts
- **Operator workload**: Driver must focus on combat, not troubleshooting control issues

This firmware solves these problems through layered safety systems, well-tested modular architecture, and careful resource management.

## Goals & Objectives

### Primary Goals
1. **Safety First**: Weapon and drive default to safe state on boot, link loss, and all error conditions
2. **Reliable Control**: Predictable, tunable holonomic drive feel with minimal drift or unexpected behavior
3. **Robustness**: Graceful degradation under voltage sag, electrical noise, and component stress
4. **Maintainability**: Modular codebase with clear phase boundaries and documented interfaces
5. **Testability**: Comprehensive bench and field tests for every subsystem

### Success Metrics
| Metric | Target | How Measured |
|--------|--------|--------------|
| Link-loss failsafe response | ≤200ms | Scope/logic analyzer on outputs |
| Weapon safety compliance | 100% (never spins unintended) | 50+ power cycles, link loss tests |
| Holonomic control accuracy | Operator satisfaction | Subjective testing on multiple axes |
| System uptime | No resets during 5min stress test | Continuous operation under load |
| Code maintainability | Pass quality gate | Embedded-quality-evaluator score |

## Scope

### In Scope (MVP - All Phases)
- **Phase 1**: Safe boot behavior and modular code structure
- **Phase 2**: CRSF receiver integration with link-loss detection
- **Phase 3**: L293D motor driver control with ramping and direction
- **Phase 4**: Holonomic mixing with normalization and tunable drive modes
- **Phase 5**: Weapon control with state-machine-based arming logic
- **Phase 6**: Self-righting servo with endpoint protection
- **Phase 7**: Full integration testing and release hardening

### In Scope (Future Enhancements)
- Field-centric drive using IMU
- Telemetry data logging
- Closed-loop motor control with encoders
- Advanced drive modes (drift, tank mode)
- Configurable profiles via EEPROM

### Out of Scope
- Autonomous navigation or path planning
- Computer vision or target tracking
- Wireless configuration/tuning interface
- Multi-robot coordination

## Technical Architecture

### Overview
The firmware follows a **modular pipeline architecture** with strict separation of concerns:

```
Input → Safety Gate → Mixing → Ramping → Actuators
         ↓
    Diagnostics
```

Each module reads from and writes to a central `RuntimeState` structure, preventing tight coupling and enabling independent testing.

### Technology Stack
| Layer | Technology | Rationale |
|-------|------------|-----------|
| Hardware | Arduino UNO (ATmega328P) | Widely available, adequate specs, proven in combat |
| Framework | Arduino + PlatformIO | Familiar API, good tooling, cross-platform builds |
| Protocol | CRSF (Crossfire) | Low-latency, robust, standard for ELRS systems |
| Motor Driver | L293D Shield | Prototype-grade, integrated, easy to replace later |
| Weapon ESC | HW30A (servo PWM) | Standard brushless ESC, proven on 2S |
| Power Architecture | Isolated rails (2S→6V drive, 5V logic) | Prevents brownouts and electrical noise coupling |

### Key Design Decisions
| Decision | Choice | Alternatives Considered | Rationale |
|----------|--------|------------------------|-----------|
| Control loop rate | 100 Hz fixed | Event-driven, faster rates | Predictable timing, adequate for human control, manageable on UNO |
| Input protocol | CRSF primary | PWM, SBUS | CRSF provides low latency, standard for ELRS, includes link quality |
| Memory strategy | Static allocation | Dynamic allocation | UNO has only 2KB RAM; predictable memory footprint critical |
| Mixing approach | Normalized holonomic | Individual motor control | Provides intuitive control, prevents saturation artifacts |
| Arming state machine | Multi-condition gate | Simple switch | Prevents accidental arming, enforces throttle-zero precondition |
| Failsafe strategy | Active monitoring + timeout | Receiver failsafe only | Defense in depth; firmware doesn't trust external devices |

### System Components

#### 1. Config Module
- Compile-time and runtime configuration
- Channel mappings, calibration values
- Drive mode profiles (beginner/normal/aggressive)
- Safety timeouts and thresholds

#### 2. Input Module
- **Custom minimal CRSF parser** optimized for UNO RAM constraints
- CRSF RC Channels packet (0x16) parsing with CRC-8-DVB-S2 validation
- 8-channel extraction from 16-channel payload (11-bit packed format)
- Input normalization to [-1.0, +1.0]
- Deadband application with hysteresis
- Link health tracking (packet timeout <200ms)
- Invalid frame rejection (CRC fail → treated as link loss)

#### 3. Safety Module
- **AVR hardware watchdog timer** for system lockup recovery (1s timeout)
- Link-loss detection via packet timeout
- Arming state machine (DISARMED ↔ ARMED) with debouncing
- Kill switch monitoring with debounce filtering
- Panic stop capability
- Output safety gating
- Error code tracking and LED-based diagnostics

#### 4. Mixer Module
- Holonomic mathematics (X, Y, R → FL, FR, RL, RR)
- Normalization to prevent saturation
- Drive mode scaling
- Optional expo curves

#### 5. Actuators Module
- L293D shield control (4 motors, direction + PWM)
- HW30A ESC control (servo-style PWM)
- Servo control (self-righting)
- Slew-rate limiting (ramping)
- Output clamping

#### 6. Diagnostics Module
- LED status indicators (SAFE/ARMED/FAILSAFE/ERROR)
- Optional serial debug output
- Test modes for bench validation

### Constraints
- **Memory**: Only 2KB SRAM; must minimize dynamic allocation and String usage
  - Phase-by-phase budget: P1=512B, P2=768B, P3=1024B, P4=1280B, P5-6=1536B, P7=1800B
  - Monitor with `-Wl,--print-memory-usage` flag
- **Processing**: 16 MHz AVR; avoid heavy floating-point in hot paths
- **Timing**: 100 Hz loop must complete in <10ms including all processing
- **Electrical**: L293D has thermal/current limits; require ramping and clamping
- **Safety**: ALL code paths must fail safe; no exceptions
- **Hardware**: Verify specific L293D shield variant for pin/timer assignments before Phase 1
- **UART**: Hardware Serial used for CRSF (420kbaud); debug via LED patterns only in production

## Pre-Implementation Requirements

**CRITICAL**: Address these before starting Phase 1 implementation:

### Hardware Verification Required
- [ ] **L293D Shield Variant**: Identify exact model (Adafruit Motor Shield v1/v2, DFRobot, generic)
- [ ] **Pin Mapping**: Document actual pins used by shield (PWM pins, direction pins)
- [ ] **Timer Allocation**: Verify timer assignments don't conflict (motors vs servo vs ESC)
- [ ] **Create**: `docs/hardware/pin_assignments.md` with verified pinout

### Design Decisions Required
- [x] **CRSF Implementation**: Custom minimal parser (decision made per gap analysis)
- [x] **Memory Strategy**: Static allocation with phase budgets (defined above)
- [x] **Safety Strategy**: Hardware watchdog + multi-layer failsafe (defined)
- [ ] **Debug Strategy**: LED patterns for production, Serial only for bench debug

### Documentation to Create
- [ ] `docs/hardware/pin_assignments.md` - Verified L293D shield pinout
- [ ] `docs/hardware/esc_calibration.md` - HW30A setup procedure
- [ ] `docs/safety/safety_requirements.md` - Formal safety requirements with test traceability
- [ ] `docs/testing/bench_tests/` - Step-by-step test procedures for each phase

**Estimated time**: 2-4 hours of research and documentation
**Status**: Must complete before Phase 1 coding begins

---

## Implementation Roadmap

### Phase 1: Project Setup & Safety Scaffolding
**Objective**: Establish safe firmware baseline where nothing moves unintentionally

**Duration Estimate**: N/A (user-driven)

#### Deliverables
- [ ] Modular code structure (config, state, safety, actuators, diagnostics modules)
- [ ] Safe boot state (all outputs neutral/off)
- [ ] Basic diagnostic LED indicating SAFE state
- [ ] Fixed-rate control loop skeleton (100 Hz)

#### Key Tasks
1. Create header/source files for each module
2. Define RuntimeState data structure
3. Implement safe output initialization
4. Create main loop with timing
5. Add LED status output

#### Dependencies
- PlatformIO project configured
- Hardware available for testing

#### Success Criteria
- Firmware compiles without errors
- All outputs verified safe at boot (scope/multimeter)
- Status LED indicates SAFE/DISARMED
- Multiple power cycles show consistent safe behavior

---

### Phase 2: Receiver Input Integration (CRSF)
**Objective**: Read control signals reliably and detect link loss

**Duration Estimate**: N/A (user-driven)

#### Deliverables
- [ ] CRSF decoder over UART
- [ ] Input normalization to [-1.0, +1.0]
- [ ] Channel mapping (8 channels: X, Y, R, Weapon, ARM, KILL, Self-Right, Mode)
- [ ] Link-loss detection (<200ms timeout)
- [ ] Deadband application

#### Key Tasks
1. Implement CRSF frame parsing
2. Extract channel values
3. Apply calibration and normalization
4. Implement link watchdog timer
5. Create input validation logic
6. Add deadband filtering

#### Dependencies
- Phase 1 complete
- ELRS receiver connected and configured
- TX16S transmitter available

#### Success Criteria
- All 8 channels reading correct values
- Link-loss detected within 200ms of signal interruption
- Inputs stable (no jitter or invalid values)
- System resumes safely after reconnection

---

### Phase 3: Motor Output Bring-up
**Objective**: Drive L293D shield outputs reliably before adding holonomic complexity

**Duration Estimate**: N/A (user-driven)

#### Deliverables
- [ ] Drive actuator module (setMotor function)
- [ ] Per-motor direction inversion flags
- [ ] Global PWM duty clamp
- [ ] Slew-rate limiter (acceleration ramping)
- [ ] Individual motor test routines

#### Key Tasks
1. Implement L293D control (direction + PWM)
2. Add per-motor configuration (inversion, limits)
3. Implement slew-rate limiting
4. Create test sweep routines
5. Validate motor directions and polarity

#### Dependencies
- Phase 2 complete
- L293D shield connected
- Motors available for testing

#### Success Criteria
- Each motor responds correctly to signed commands (-1.0 to +1.0)
- No resets during motor operation
- Ramping reduces current spikes (observable via voltage sag)
- Motor polarity documented and correct

---

### Phase 4: Holonomic Mixing & Drive Feel Tuning
**Objective**: Implement predictable holonomic control

**Duration Estimate**: N/A (user-driven)

#### Deliverables
- [ ] Mixer module with holonomic math
- [ ] Normalization logic to prevent clipping
- [ ] Drive mode profiles (beginner/normal/aggressive)
- [ ] Expo curves (optional)
- [ ] Tunable axis scaling

#### Key Tasks
1. Implement holonomic mixing matrix (X, Y, R → motors)
2. Add normalization to preserve motion ratios
3. Create drive mode profile system
4. Implement mode switching via CH8
5. Test all motion combinations (translate, rotate, diagonal)
6. Tune parameters for feel

#### Dependencies
- Phase 3 complete
- Bot available for off-ground and on-ground testing

#### Success Criteria
- Pure X, Y, R motions behave as expected
- Combined motions smooth and predictable
- Mode switching changes feel appropriately
- Minimal drift at stick center
- Rotation doesn't cause unwanted translation

---

### Phase 5: Weapon Control & Arming State Machine
**Objective**: Safe, predictable weapon control

**Duration Estimate**: N/A (user-driven)

#### Deliverables
- [ ] Weapon control module (PWM to HW30A)
- [ ] Arming state machine (DISARMED ↔ ARMED)
- [ ] Arming conditions (switch + throttle-zero + link-ok)
- [ ] Weapon soft-start ramping
- [ ] Failsafe disarm logic

#### Key Tasks
1. Implement servo-style PWM output for ESC
2. Create arming state machine
3. Enforce arming preconditions (SA switch middle, weapon throttle near zero)
4. Implement weapon ramping
5. Add disarm on link-loss/kill switch
6. Test fault injection (reboot while armed)

#### Dependencies
- Phase 4 complete
- HW30A ESC and weapon motor available
- Weapon mechanically secured or removed for safety

#### Success Criteria
- Weapon never spins unintentionally
- Cannot arm without throttle at zero
- Link loss disarms within timeout
- Reboot always results in disarmed state
- Kill switch immediately disarms

---

### Phase 6: Self-Righting Servo Control & Limits
**Objective**: Reliable self-righting with mechanical protection

**Duration Estimate**: N/A (user-driven)

#### Deliverables
- [ ] Servo control module
- [ ] Endpoint calibration and limits
- [ ] Momentary control mode (button hold)
- [ ] Servo speed limiting
- [ ] Failsafe behavior (neutral on link loss)

#### Key Tasks
1. Implement servo control via Arduino Servo library
2. Calibrate safe endpoint limits
3. Implement rate limiting (slow movement to endpoints)
4. Add momentary button logic (CH7)
5. Define failsafe position (neutral)
6. Test under full system load

#### Dependencies
- Phase 5 complete
- Servo mechanism available

#### Success Criteria
- Servo moves to commanded positions
- No hard binding or stalls
- No brownouts during servo operation
- Failsafe returns servo to neutral
- Operator control intuitive

---

### Phase 7: Integration, Stress Testing, and Release Hardening
**Objective**: Convert bench-tested firmware to competition-ready system

**Duration Estimate**: N/A (user-driven)

#### Deliverables
- [ ] Consolidated configuration file
- [ ] LED status patterns documented
- [ ] Match mode build configuration
- [ ] Test harness routines (motor sweep, input monitor, failsafe sim)
- [ ] Quick start and troubleshooting documentation

#### Key Tasks
1. Consolidate all configuration into single header
2. Implement comprehensive LED status patterns
3. Create diagnostic test modes
4. Execute electrical stress tests (rapid direction changes, concurrent loads)
5. Execute RF stress tests (range, link loss, interference)
6. Execute thermal tests (extended operation)
7. Document wiring, LED meanings, troubleshooting

#### Dependencies
- Phases 1-6 complete
- Full bot assembled
- Competition environment available for testing

#### Success Criteria
- 20+ consecutive safe power cycles
- 5+ minutes continuous operation without resets
- Consistent failsafe response under all tested conditions
- L293D temperature acceptable after extended use
- All safety requirements verified
- Operator documentation complete

---

## Risk Assessment

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| L293D thermal failure under stall | Medium | High | Voltage limiting, duty clamps, acceleration ramping, thermal monitoring |
| UNO brownout from voltage sag | Medium | High | Isolated logic rail via dedicated BEC, bulk capacitors on motor rail |
| Weapon spins on reboot | Low | Critical | Initialize weapon PWM before any other code, enforce safe defaults |
| CRSF parsing errors cause glitches | Low | Medium | Strict CRC/length validation, treat invalid frames as link loss |
| Timer/resource contention (Servo + PWM) | Medium | Medium | Careful timer allocation, test full system concurrently |
| Omni wheel geometry different than assumed | Medium | Low | Provide calibration routine, adjustable mixing matrix |
| Memory exhaustion (2KB SRAM limit) | Medium | Medium | Static allocation, minimize Strings, monitor usage with build reports |
| Electrical noise causes receiver glitches | Low | Medium | Separate power rails, star grounding, bulk capacitors, twisted pairs |
| Arming state machine edge cases | Low | Critical | Comprehensive state transition testing, fault injection tests |

## Open Questions

- [x] Channel mapping confirmed (8 channels defined)
- [x] Power architecture finalized (2S split to 6V drive, 5V logic)
- [x] CRSF vs PWM input (CRSF primary)
- [ ] ESC calibration required? (determine during Phase 5)
- [ ] Backup arming method in case SA switch fails?
- [ ] Telemetry/logging requirements for competition?

## Progress Tracking

### Current Status
**Active Phase**: Not started (awaiting `/start-work`)
**Overall Progress**: 0%

### Phase Status
| Phase | Status | Progress | Notes |
|-------|--------|----------|-------|
| Phase 1: Project Setup & Safety Scaffolding | Not Started | 0% | - |
| Phase 2: Receiver Input Integration | Not Started | 0% | - |
| Phase 3: Motor Output Bring-up | Not Started | 0% | - |
| Phase 4: Holonomic Mixing & Drive Tuning | Not Started | 0% | - |
| Phase 5: Weapon Control & Arming | Not Started | 0% | - |
| Phase 6: Self-Righting Servo Control | Not Started | 0% | - |
| Phase 7: Integration & Stress Testing | Not Started | 0% | - |

### Changelog
| Date | Change | Reason |
|------|--------|--------|
| 2025-12-25 | Initial Master Plan created | Project initialization via /new-project |

---

**Status**: active
**Created**: 2025-12-25
**Author**: Claude Sonnet 4.5 + /new-project command
