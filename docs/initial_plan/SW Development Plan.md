# 🧭 Battlebot Firmware Masterplan (Holonomic Drive) 🤖⚔️

## 1) Scope & Goals ✅
This firmware controls a small battlebot with:
- **Holonomic drive** (4 independent TT motors on omni wheels) 🛞↔️↕️🔄  
- **Weapon** (brushless motor via HW30A ESC) ⚔️  
- **Self-righting** (servo) 🔄  
- **Radio control** via **TX16S → ELRS receiver → Arduino UNO** 📡🧠  

Primary goals (in priority order):
1. **Safety**: weapon and drive default safe on boot; failsafe on link loss.
2. **Control quality**: predictable holonomic translation + rotation; tunable feel.
3. **Robustness**: stable operation under voltage sag/noise; graceful degradation.
4. **Maintainability**: modular codebase with clear phase boundaries.
5. **Testability**: structured bench tests and on-floor tests for every phase.

Non-goals (explicitly out of scope for this plan):
- Autonomous navigation
- Closed-loop motor speed control (encoders)
- Advanced IMU fusion/field-centric drive (future phase)

---

## 2) System Context 🧩
### Hardware assumptions
- **MCU**: Arduino UNO (ATmega328P constraints: limited RAM/CPU/timers/serial)
- **Drive**: L293D shield driving 4 TT motors (prototype-grade current margins)
- **Weapon**: HW30A ESC receiving servo-style PWM
- **Servo**: self-righting action
- **Power**: split rails recommended (drive rail vs logic rail) to reduce resets

### Control assumptions
- **Holonomic mixing** performed in firmware.
- Radio input via **CRSF (preferred)** or PWM fallback (if receiver supports).
- Soft-start and output clamps mitigate L293D stress.

---

## 3) Top-Level Architecture 🏗️
### Core modules
1. **Config**  
   - Channel mapping, calibration, limits, ramps, deadbands.
2. **Input**  
   - CRSF decoder (primary) and PWM reader (fallback).
3. **Safety**  
   - Link watchdog, failsafe, weapon arming state machine, panic stop.
4. **Mixer**  
   - Holonomic math, normalization, saturation management.
5. **Actuators**  
   - Drive motor outputs (direction + PWM), weapon PWM, servo control.
6. **Diagnostics**  
   - LED/status reporting, optional serial debug, test modes.

### Runtime model (loop)
- Fixed-rate control loop (e.g., 100 Hz)  
- Each tick:
  1) Read input  
  2) Validate + update watchdog  
  3) Safety gating (failsafe/arm)  
  4) Holonomic mix  
  5) Apply ramps/clamps  
  6) Output to motors/ESC/servo  
  7) Update status indicators

---

## 4) Development Strategy 🧪
### Principles
- **Incremental integration**: one risk dimension at a time.
- **Bench-first**: validate signals before spinning motors.
- **Safe defaults**: every new feature must fail safe.
- **Feature flags**: compile-time or config flags for test modes.

### Deliverable structure
- A **phase gate** means: the phase’s tests pass and regressions are ruled out.
- Each phase produces:
  - code artifacts (modules)
  - config updates
  - a test checklist
  - a short “known limitations” section

---

## 5) Phases Overview 🗺️
This master plan is implemented as separate phase documents:

- **Phase 1 — Project Setup & Safety Scaffolding** 🧯
- **Phase 2 — Receiver Input Integration (CRSF/PWM)** 📡
- **Phase 3 — Motor Output Bring-up (Single → Four Motors)** 🛞
- **Phase 4 — Holonomic Mixing & Drive Feel Tuning** ↔️↕️🔄
- **Phase 5 — Weapon Control & Arming State Machine** ⚔️🛑
- **Phase 6 — Self-Righting Servo Control & Limits** 🔄
- **Phase 7 — Integration, Stress Testing, and Release Hardening** 🧪✅

---

## 6) Definition of Done 🎯
Firmware is considered ready for “real bot use” when:
- Link-loss failsafe consistently stops **drive + weapon** within ≤200 ms.
- Weapon cannot spin without explicit arm action and throttle-zero precondition.
- Holonomic motions are predictable and tunable; no unexpected axis coupling.
- No resets observed in representative driving + weapon spin tests.
- A minimal field troubleshooting guide exists (LED patterns, checks).

---

## 7) Known Risks (tracked across phases) ⚠️
- **L293D current/thermal limits** under stall/reversal events.
- **UNO timer/resource contention** (servo library + PWM + serial parsing).
- **Power integrity** (grounding, dips, noise) causing MCU resets.
- **Receiver protocol variance** (CRSF vs PWM, wiring differences).
- **Weapon safety** (arming edge cases, reboot behavior).

Each phase document includes mitigation steps and acceptance tests that reduce these risks progressively.

# Phase 1 — Project Setup & Safety Scaffolding 🧯🤖

## Objective ✅
Establish a safe firmware baseline where **nothing moves unintentionally**, and the codebase is structured for the later holonomic features without becoming a single file “blob”.

---

## Deliverables 📦
### Code structure (minimum viable modularity)
- `config.h/.cpp`: constants and tunables
- `state.h/.cpp`: runtime state (inputs, outputs, timers, flags)
- `safety.h/.cpp`: failsafe defaults, watchdog, arming skeleton
- `actuators.h/.cpp`: stubs for drive/weapon/servo outputs
- `diagnostics.h/.cpp`: LED + optional serial logging

### Safety baseline behaviors
- Boot state:
  - Drive motors OFF (PWM=0, directions neutral)
  - Weapon output = minimum throttle
  - Weapon armed = false
  - Servo at safe neutral position
- “No input” state:
  - same as boot (safe hold)

---

## Implementation Notes 🧠
### Loop timing
- Decide fixed loop frequency (e.g., **100 Hz**) now.
- Use `millis()` for coarse timing; avoid heavy `delay()` usage.
- Keep one central loop that calls module functions in strict order.

### Output neutrality
- For L293D, “stop” can mean different electrical states depending on how you drive inputs.
  - Start with the safest: **PWM=0** for all channels.
  - Defer “active braking” strategies until later; braking can increase current spikes.

### Debug hygiene
- Provide a `DEBUG` compile flag.
- In normal mode, avoid continuous Serial prints (UNO limitations).

---

## Tests ✅ (Bench-only)
- Power Arduino + logic rail with motors disconnected.
- Confirm:
  - status LED indicates “SAFE / DISARMED”
  - weapon PWM pin outputs minimum throttle pulses continuously
- Power cycle several times:
  - no transient “blips” on outputs (if you have a scope/logic analyzer, verify)

---

## Risks & Mitigations ⚠️
- **Risk:** accidental high output during initialization  
  **Mitigation:** initialize outputs first, then initialize input modules.
- **Risk:** code structure still devolves into monolithic file  
  **Mitigation:** enforce module boundaries early, even if functions are small.

---

## Exit Criteria 🎯
- Firmware compiles cleanly.
- All outputs are known-safe at boot.
- Basic diagnostics confirm safe state without requiring a PC connected.


# Phase 2 — Receiver Input Integration (CRSF Primary / PWM Fallback) 📡🧠

## Objective ✅
Read control signals reliably from the ELRS receiver, detect link loss, and deliver **normalized inputs** to the rest of the system.

---

## Deliverables 📦
### Input module
- CRSF decoder over UART (preferred)
- PWM input reader fallback (if needed)
- Input normalization to `[-1.0 .. +1.0]` plus discrete switches

### Channel map (documented)
Example logical channels:
- `X` strafe (left/right)
- `Y` forward/back
- `R` rotate (yaw)
- `ARM` weapon arm switch
- `W` weapon throttle (knob/trigger)
- `SR` self-right command (button/switch)
- `KILL` optional hard kill switch

---

## Implementation Notes 🧠
### CRSF specifics
- UART receive must be non-blocking.
- Parse frames and extract channel values.
- Provide a “last valid frame timestamp”.

### PWM fallback specifics
- PWM decoding on UNO can be done via:
  - `pulseIn()` (simple, but can block—use only if absolutely necessary)
  - pin-change interrupts + timer capture (better, more complex)
- If PWM fallback is used, constrain to minimum channels required.

### Validation & deadband
- Validate channel ranges (e.g., 988–2012 µs equivalent if PWM).
- Apply deadband around center to prevent creeping.

---

## Tests ✅
### Bench tests
- With receiver connected:
  - confirm inputs update at expected rate
  - confirm all channels map correctly
- Link-loss:
  - disconnect receiver signal → confirm “input stale” within timeout
- Reconnect:
  - confirm inputs resume and weapon remains disarmed

---

## Risks & Mitigations ⚠️
- **Risk:** UNO UART resource conflicts with debugging  
  **Mitigation:** keep Serial prints minimal; consider debug over LED patterns.
- **Risk:** CRSF frame parsing errors cause random outputs  
  **Mitigation:** strict CRC/length checks; reject invalid frames; treat as link loss.

---

## Exit Criteria 🎯
- Input values stable and correct.
- Link-loss detection reliable and fast (target ≤150 ms).
- No blocking operations in normal runtime loop.


# Phase 3 — Motor Output Bring-up (Single → Four Motors) 🛞🔧

## Objective ✅
Drive the L293D shield outputs in a controlled, consistent way and verify motor directions, wiring, and basic performance—before adding holonomic mixing complexity.

---

## Deliverables 📦
### Drive actuator module
- `setMotor(i, cmd)` where `cmd ∈ [-1.0..+1.0]`
- Per-motor:
  - direction inversion flag
  - PWM scaling limit
- Global:
  - max duty clamp (start conservative)
  - slew-rate limiter (accel ramp)

---

## Implementation Notes 🧠
### L293D considerations
- L293D is not efficient; heat and voltage drop are expected.
- Use conservative PWM to reduce stress.
- Prefer “coast to stop” first (PWM=0), add braking only if needed.

### Slew-rate limiting (recommended)
- Prevent instant reversals and current spikes:
  - clamp delta per loop (e.g., `0.03` at 100 Hz ≈ 0→100% in ~0.33s)
- Apply limiter after mixing but before output mapping.

---

## Tests ✅
### Step-by-step
1) Connect one motor to M1:
   - forward at 20%, 40%, 60%
   - reverse same
   - verify stop
2) Repeat for M2–M4
3) Connect all four motors:
   - run each motor independently from a test routine
   - ensure no resets

### Abuse test (prototype-safe)
- Briefly “stall” wheel by hand at low PWM
- Observe if shield overheats quickly or MCU resets

---

## Risks & Mitigations ⚠️
- **Risk:** L293D overheats at stall  
  **Mitigation:** clamps + ramps + lower motor voltage rail.
- **Risk:** motor polarity confusion later breaks holonomic math  
  **Mitigation:** standardize motor indexing and record inversion flags now.

---

## Exit Criteria 🎯
- Each motor responds correctly to signed command input.
- No unexpected resets during low/moderate duty tests.
- Ramping and clamps demonstrably reduce abrupt current spikes.


# Phase 4 — Holonomic Mixing & Drive Feel Tuning ↔️↕️🔄🎮

## Objective ✅
Implement holonomic control such that the bot can:
- translate forward/back (Y)
- strafe left/right (X)
- rotate (R)
…and combinations of these feel predictable and tunable.

---

## Deliverables 📦
### Mixer module
- Inputs: `X`, `Y`, `R` in normalized `[-1..+1]`
- Outputs: `FL`, `FR`, `RL`, `RR` in `[-1..+1]`
- Normalization/saturation logic to prevent clipping artifacts
- Configuration:
  - deadbands
  - expo curves (optional)
  - axis scaling weights (e.g., reduce rotation sensitivity)

### Control “modes” (optional)
- Mode switch (e.g., “beginner” vs “aggressive”)
- Per-mode max duty and ramp values

---

## Holonomic Math (baseline) 🧮
Typical mecanum-style mixing matrix (also a good starting point for omni holonomic):
- FL = Y + X + R
- FR = Y − X − R
- RL = Y − X + R
- RR = Y + X − R

Then normalize by dividing all by `max(|FL|,|FR|,|RL|,|RR|, 1.0)`.

⚠️ Wheel orientation matters. If behavior is rotated/mirrored, adjust signs or swap motor indices.

---

## Implementation Notes 🧠
### Normalization strategy
- Without normalization, diagonal moves saturate prematurely.
- With normalization, you preserve the ratio of components and keep control smooth.

### Axis tuning
- TT motors + L293D can feel “snappy” near center.
- Deadband + light expo improves feel.
- Consider limiting `R` more than `X/Y` if rotation dominates.

### Preventing drift
- Ensure deadband is applied before mixing.
- Ensure calibration offsets are applied (center trim).

---

## Tests ✅
### Off-ground tests
- Pure motions:
  - X only → strafe
  - Y only → forward/back
  - R only → rotate in place
- Combined motions:
  - forward + rotate
  - strafe + rotate
  - diagonal + rotate

### On-ground tests (low power)
- Confirm minimal unintended drift at center
- Confirm “rotate in place” does not also translate

---

## Risks & Mitigations ⚠️
- **Risk:** omni wheel geometry differs from assumed matrix  
  **Mitigation:** provide a calibration routine or mapping adjustments.
- **Risk:** clipping causes weird non-linear behavior  
  **Mitigation:** normalization + clamp after normalization (not before).

---

## Exit Criteria 🎯
- Holonomic control behaves as expected in all three axes.
- User-adjustable tuning parameters exist and are documented.
- No runaway behavior when sticks return to center.


# Phase 5 — Weapon Control & Arming State Machine ⚔️🛑🧯

## Objective ✅
Make the weapon system safe, predictable, and resilient to link loss and reboots.

---

## Deliverables 📦
### Weapon control module
- Outputs PWM to HW30A ESC (servo-style)
- Implements:
  - min/max pulse configuration
  - ramp limiter (soft-start)
  - throttle clamp for testing

### Arming state machine
States:
- DISARMED
- ARMED

Rules:
- Only arm if:
  - link healthy
  - ARM switch active
  - weapon throttle near zero
- Disarm if:
  - link unhealthy
  - ARM switch off
  - panic/kill asserted
- On boot: DISARMED always

---

## Implementation Notes 🧠
### ESC behavior
- Some ESCs require a stable “min throttle” signal for initialization.
- Provide an optional “ESC calibration mode” (manual invocation) that:
  - outputs max then min per instructions (only when explicitly requested).

### Soft-start
- Ramp weapon throttle slower than drive to reduce current spikes.
- Enforce a minimum “spin-up delay” if needed for safety (optional).

---

## Tests ✅
### Safe bench tests
- ESC connected, motor NOT mechanically loaded, weapon removed if applicable.
- Verify:
  - cannot spin without arm
  - arm requires throttle=0
  - link loss → weapon stops quickly

### Fault injection
- Reset Arduino while armed:
  - ensure weapon immediately returns to min throttle and disarms

---

## Risks & Mitigations ⚠️
- **Risk:** weapon spins on reboot if ESC interprets garbage  
  **Mitigation:** initialize weapon PWM output immediately, before other init.
- **Risk:** throttle noise around zero blocks arming  
  **Mitigation:** deadband for weapon input; explicit threshold.

---

## Exit Criteria 🎯
- Weapon never spins unintentionally.
- Failsafe stops weapon reliably within timeout.
- Arming rules enforced under all tested conditions.


# Phase 6 — Self-Righting Servo Control & Limits 🔄🦾

## Objective ✅
Implement reliable self-righting servo control with mechanical protection and clear operator behavior.

---

## Deliverables 📦
- Servo control module:
  - discrete positions or continuous control
  - endpoint limits
  - optional speed limiting
- Behavior selection:
  - Toggle mode (switch changes position)
  - Momentary mode (button hold actuates)

---

## Implementation Notes 🧠
### Servo endpoints
- Hard endpoints can stall the servo and cause brownouts.
- Calibrate safe min/max pulses.
- Consider a “soft approach” to endpoints:
  - ramp servo position over 0.2–0.5s

### Interaction with failsafe
- Decide policy:
  - on failsafe: servo goes to neutral
  - or servo holds last safe position
- Default recommended: neutral on failsafe.

---

## Tests ✅
- With bot lifted:
  - command each position
  - verify no hard binding
- With full system powered:
  - activate servo while driving slowly
  - ensure no MCU resets

---

## Risks & Mitigations ⚠️
- **Risk:** servo stall causes logic rail dip  
  **Mitigation:** adequate 5V BEC, endpoint limits, soft movement.
- **Risk:** servo noise affects receiver/MCU  
  **Mitigation:** wiring discipline, separate rails.

---

## Exit Criteria 🎯
- Servo action works consistently.
- Endpoints safe; no repeated brownouts.
- Operator behavior is intuitive and documented.


# Phase 7 — Integration, Stress Testing, and Release Hardening 🧪✅🧯

## Objective ✅
Convert a “works on the bench” firmware into a reliable integrated system that tolerates real driving conditions and repeated power cycles.

---

## Deliverables 📦
### Integration hardening
- Consolidated configuration file with documented defaults
- LED status patterns:
  - SAFE/DISARMED
  - ARMED
  - FAILSAFE
  - INPUT ERROR
- “Match mode” build:
  - debug disabled
  - aggressive safety gating enabled
  - conservative clamps for L293D

### Test harness routines
- Motor test sweep
- Input monitor mode
- Failsafe simulation mode

---

## Stress Tests ✅
### Electrical stress
- Rapid direction changes at moderate duty
- Weapon spin-up while driving
- Servo activation while driving
- Power cycle tests:
  - 20 consecutive reboots
  - confirm no weapon spin on boot

### RF stress
- Range test in realistic environment
- Link-loss tests:
  - power off transmitter
  - unplug receiver signal
  - walk out of range
- Confirm failsafe response is consistent

### Thermal observation
- After 2–5 minutes of driving:
  - inspect L293D shield temperature
  - if too hot to touch, reduce duty/clamps and consider airflow.

---

## Documentation 📓
- “Quick start”:
  - wiring verification steps
  - LED meanings
  - arming procedure
- “Troubleshooting”:
  - symptoms → likely causes → actions

---

## Risks & Mitigations ⚠️
- **Risk:** L293D fails in extended use  
  **Mitigation:** document upgrade path; tune clamps; limit stalls.
- **Risk:** hidden race conditions in input parsing  
  **Mitigation:** sanity checks, reject invalid frames, fail-safe on parse errors.

---

## Exit Criteria 🎯
- Passes full stress checklist without unsafe behavior.
- Predictable holonomic control at chosen duty limit.
- Weapon safety is demonstrably robust to link loss and reboots.
- A short user/operator guide exists.