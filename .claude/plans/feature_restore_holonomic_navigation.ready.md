# Feature Plan: Restore Holonomic Navigation After AFMotor Integration

## Overview
Restore the Phase 4 holonomic mixing functionality that was temporarily replaced with sequential motor tests during AFMotor library integration. This will bring back full operator control with X/Y translation and rotation.

## Current State Analysis

### What We Have ✓
- **AFMotor library integration complete** - all 4 motors working with correct pin assignments
- **Motor polarity inversions configured** - M1, M2, M3 polarities verified
- **Holonomic mixing math still present** - `apply_expo()`, `normalize_outputs()`, `update_mode_params()` all intact
- **CRSF input processing working** - Phase 2 complete, receiving stick inputs
- **Safety systems operational** - arming state machine, link-loss detection
- **Weapon and servo control** - Phase 5 & 6 complete

### What's Missing ✗
- **`mixing_update()` replaced with test code** - currently runs sequential motor test instead of reading stick inputs
- **No holonomic mixing active** - X/Y/R inputs not being converted to motor commands
- **Drive modes not functional** - mode switching exists but not connected to actual mixing

### Root Cause
During AFMotor debugging, `mixing_update()` was temporarily replaced with hardcoded motor test sequences to isolate hardware issues. The original holonomic mixing implementation needs to be restored.

## Requirements

### FR-1: Restore Holonomic Mixing Logic
- Read normalized inputs from `g_state.input` (x_axis, y_axis, r_axis)
- Apply expo curves based on current drive mode
- Calculate motor outputs using holonomic mixing equations:
  ```
  FL = Y + X + R
  FR = Y - X - R
  RL = Y - X + R
  RR = Y + X - R
  ```
- Apply normalization to prevent saturation
- Scale outputs by drive mode max_duty

### FR-2: Integrate with CRSF Input
- Use existing `g_state.input.rc_channels[]` data
- Map CRSF channels to X/Y/R axes per Phase 2 spec
- Respect deadband settings (INPUT_DEADBAND = 5%)
- Handle link-loss failsafe (motors to zero when link lost)

### FR-3: Drive Mode Support
- Support Beginner/Normal/Aggressive modes
- Apply correct expo and max_duty per mode
- Enable runtime mode switching via aux channel
- Default to NORMAL mode on boot

### FR-4: Safety Integration
- Respect arming state - motors zero when SAFE
- Link-loss detection - motors stop on timeout
- Emergency stop capability

## Implementation Plan

### Step 1: Save Test Code for Reference
**Action**: Comment out sequential test, save for hardware validation use
**Files**: `src/mixing.cpp`
**Rationale**: We may need this test code again for future debugging

### Step 2: Restore Original Holonomic Mixing
**Action**: Implement proper `mixing_update()` with holonomic math
**Files**: `src/mixing.cpp`
**Code Structure**:
```cpp
void mixing_update() {
  // Read inputs from global state
  float x = g_state.input.x_axis;
  float y = g_state.input.y_axis;
  float r = g_state.input.r_axis;

  // Apply expo curves
  x = apply_expo(x, g_mode_params.expo);
  y = apply_expo(y, g_mode_params.expo);
  r = apply_expo(r * ROTATION_SCALE, g_mode_params.expo);

  // Holonomic mixing
  float fl = y + x + r;
  float fr = y - x - r;
  float rl = y - x + r;
  float rr = y + x - r;

  // Normalize to prevent saturation
  normalize_outputs(&fl, &fr, &rl, &rr);

  // Scale by max duty and convert to motor PWM
  int16_t fl_pwm = (int16_t)(fl * g_mode_params.max_duty);
  int16_t fr_pwm = (int16_t)(fr * g_mode_params.max_duty);
  int16_t rl_pwm = (int16_t)(rl * g_mode_params.max_duty);
  int16_t rr_pwm = (int16_t)(rr * g_mode_params.max_duty);

  // Send to actuators
  actuators_set_motor(0, rl_pwm);  // M1: Rear-Left
  actuators_set_motor(1, rr_pwm);  // M2: Rear-Right
  actuators_set_motor(2, fl_pwm);  // M4: Front-Left
  actuators_set_motor(3, fr_pwm);  // M3: Front-Right
}
```

### Step 3: Verify Input Mapping
**Action**: Check CRSF channel mapping matches expected axes
**Files**: `src/input.cpp`
**Validation**:
- Channel 1 (Roll) → X-axis (strafe left/right)
- Channel 2 (Pitch) → Y-axis (forward/backward)
- Channel 4 (Yaw) → R-axis (rotate)
- Ensure proper normalization to [-1.0, +1.0]

### Step 4: Add Drive Mode Switching (Optional)
**Action**: Map aux channel to drive mode selection
**Files**: `src/input.cpp`, `src/mixing.cpp`
**Note**: Can be deferred - default NORMAL mode is sufficient for initial testing

### Step 5: Bench Testing Protocol
**Action**: Test on bench before field use
**Tests**:
1. **Pure Y (forward/backward)** - all motors same direction
2. **Pure X (strafe)** - diagonal pairs opposite
3. **Pure R (rotate)** - left/right sides opposite
4. **Combined motions** - smooth transitions
5. **Deadband** - no drift at stick center
6. **Normalization** - no weird behavior at full stick deflection
7. **Link-loss** - motors stop when TX powered off
8. **Mode switching** - feel changes between modes (if implemented)

### Step 6: Motor Direction Verification
**Action**: Verify holonomic mixing directions match physical layout
**Expected**:
- **Forward stick**: All motors forward
- **Right stick**: FL+RR forward, FR+RL backward
- **Rotate right**: Left side forward, right side backward

**Adjustment**: If directions wrong, adjust polarity flags in config.h

## Files to Modify

### Primary Changes
| File | Changes | Lines Affected |
|------|---------|----------------|
| `src/mixing.cpp` | Restore holonomic mixing logic in `mixing_update()` | ~107-136 (30 lines) |

### Optional Changes
| File | Changes | Lines Affected |
|------|---------|----------------|
| `src/input.cpp` | Add drive mode switching logic | TBD |
| `src/mixing.cpp` | Add mode switch handling | TBD |

## Success Criteria

### SC-1: Pure Axis Motion
- Forward stick → all motors forward at equal speed
- Strafe right → FL+RR forward, FR+RL backward
- Rotate right → left side forward, right side backward

### SC-2: Combined Motion
- Forward + Strafe → diagonal movement, all motors running
- Forward + Rotate → arc motion, smooth speed variation
- Strafe + Rotate → orbital motion around robot center

### SC-3: No Drift
- Sticks centered → all motors stopped (within deadband tolerance)
- Small stick movements → proportional motor response

### SC-4: Saturation Handling
- Full stick + rotate → motors scale proportionally (no clipping)
- Normalization preserves motion direction

### SC-5: Safety Compliance
- Motors zero when SAFE
- Motors stop on link loss
- Emergency stop works

## Risks & Mitigations

### Risk 1: Motor Direction Mismatch
**Impact**: Robot moves unexpectedly (wrong direction or rotation)
**Likelihood**: Medium
**Mitigation**:
- Start with low throttle testing
- Verify each axis independently before combining
- Use motor polarity flags to correct without code changes

### Risk 2: Normalization Issues
**Impact**: Weird behavior at full stick deflection
**Likelihood**: Low (math already tested in Phase 4)
**Mitigation**:
- Test full stick combinations on bench
- Add logging/telemetry if issues arise

### Risk 3: AFMotor Library Interaction
**Impact**: Motor control timing or behavior different than Phase 4
**Likelihood**: Low (AFMotor handles PWM/direction, we just set commands)
**Mitigation**:
- Test incrementally (one axis at a time)
- Monitor for any stuttering or unexpected behavior

## Rollback Plan

If holonomic mixing fails:
1. **Immediate**: Revert `mixing.cpp` to sequential test (via git)
2. **Diagnose**: Use sequential test to verify motors still working
3. **Debug**: Add telemetry to see what mixing is calculating
4. **Iterate**: Fix issues and re-test

## Memory Impact

**Expected Change**: Negligible
- No new data structures
- No dynamic allocation
- Same code footprint as Phase 4 (already compiled once)

**Current Budget**:
- RAM: 454/2048 bytes (22.2%) - Phase 7 budget is 1800 bytes
- Flash: 9184/32256 bytes (28.5%)

**Margin**: Excellent - 1346 bytes RAM headroom, 23072 bytes Flash headroom

## Testing Checklist

### Pre-Test Verification
- [ ] Code compiles without errors
- [ ] Memory usage within budget
- [ ] Robot on blocks (wheels off ground)
- [ ] Battery voltage nominal (>11V for 3S)
- [ ] CRSF link established

### Bench Tests (on blocks)
- [ ] Forward stick → all wheels forward
- [ ] Backward stick → all wheels backward
- [ ] Right strafe → diagonal pairs correct
- [ ] Left strafe → diagonal pairs correct
- [ ] Rotate right → left forward, right backward
- [ ] Rotate left → right forward, left backward
- [ ] Sticks centered → wheels stopped
- [ ] Combined forward+strafe → diagonal motion
- [ ] Combined forward+rotate → arc motion
- [ ] Full stick deflection → smooth scaling

### Safety Tests
- [ ] Power on → motors zero until armed
- [ ] Arming → motors respond to sticks
- [ ] Disarm → motors stop immediately
- [ ] Link loss → motors stop within 200ms
- [ ] Emergency stop works

### Field Tests (if bench tests pass)
- [ ] Gentle forward/backward motion
- [ ] Gentle strafing motion
- [ ] Gentle rotation
- [ ] Figure-8 pattern
- [ ] Stop and start smoothly
- [ ] No unexpected behavior

## Timeline Estimate

| Task | Duration | Dependencies |
|------|----------|--------------|
| Code restoration | 15 min | None |
| Compile & upload | 5 min | Code complete |
| Bench testing | 30 min | Upload complete |
| Direction tuning | 15 min | Bench tests done |
| Field validation | 30 min | Bench tests pass |
| **Total** | **~1.5 hours** | |

## Dependencies

### Prerequisites
- ✅ AFMotor integration complete
- ✅ All 4 motors verified working
- ✅ Motor polarity inversions configured
- ✅ CRSF input processing operational
- ✅ Safety systems functional

### Blockers
- None - all prerequisites met

## Follow-up Work

After holonomic navigation restored:
1. **Fine-tune drive feel** - adjust expo, rotation scale
2. **Add drive mode switching** - implement aux channel control
3. **Field-centric mode** (future) - add IMU integration
4. **Remove old code** - clean up deprecated shift register code
5. **Update documentation** - reflect AFMotor pin changes

## Notes

### Why This Matters
The sequential motor test served its purpose - we validated AFMotor library integration and fixed pin conflicts. Now we need to restore operator control to make the robot actually drivable. This is the difference between a motor test jig and a functional battlebot.

### Confidence Level
**HIGH** - The holonomic mixing math was already proven in Phase 4. We're just reconnecting existing, tested components. The only unknowns are potential direction mismatches, which are easily corrected with polarity flags.

---

**Status**: ready
**Created**: 2025-12-29
**Author**: feature-architect (orchestrator invocation)
**Phase**: 7 (AFMotor Integration Cleanup)
**Priority**: HIGH - Required for robot operation
