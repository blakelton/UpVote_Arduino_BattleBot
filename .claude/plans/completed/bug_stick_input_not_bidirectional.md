# Bug Troubleshooting: Stick Inputs Not Bidirectional

## Problem Statement

**CRITICAL**: All three control axes show **unidirectional behavior** - moving the stick in EITHER direction causes the SAME robot motion instead of opposite motions.

### Observed Behavior

| Stick Action | Expected Behavior | Actual Behavior | Status |
|--------------|-------------------|-----------------|--------|
| Right stick FORWARD | Bot moves forward | Bot moves **backward** | ❌ Wrong + unidirectional |
| Right stick BACKWARD | Bot moves backward | Bot moves **backward** | ❌ SAME as forward! |
| Right stick LEFT | Bot strafes left | Bot strafes **left** | ❌ Wrong + unidirectional |
| Right stick RIGHT | Bot strafes right | Bot strafes **left** | ❌ SAME as left! |
| Left stick LEFT | Bot rotates left | Bot rotates **right** | ❌ Wrong + unidirectional |
| Left stick RIGHT | Bot rotates right | Bot rotates **right** | ❌ SAME as left! |

**KEY PATTERN**: Each axis responds in ONLY ONE direction regardless of stick position.

## What We Know FOR CERTAIN

### ✅ Verified Working
1. **AFMotor library control** - All 4 motors respond to PWM commands correctly
2. **Motor bidirectional control** - Motors can spin both forward AND backward when commanded
3. **Sign preservation in motor control** - `actuators.cpp` correctly handles negative PWM values
4. **Holonomic mixing math** - Equations are correct (FL=y+x+r, FR=y-x-r, etc.)
5. **normalize_channel() math** - Verified: (raw-992)/(1811-992) produces correct -1.0 to +1.0 range
6. **Duty cycle scaling** - Multiplying float by positive constant preserves sign
7. **CRSF link is established** - Receiving packets, link_ok = true

### ❌ Proven NOT Working
1. **Bidirectional input** - Stick movements in opposite directions produce SAME values

### 🔍 Diagnostic Test Result
**Test**: Direct pitch input → M1 motor only (no mixing, no expo)
```cpp
float y = g_state.input.pitch;
int16_t test_pwm = (int16_t)(y * 200);
actuators_set_motor(0, test_pwm);  // M1 only
```
**Result**: "Motor spins the SAME DIRECTION both ways"

**Conclusion**: Problem is definitively in CRSF input parsing or TX16S configuration, NOT in mixing or motor control.

## Root Cause Hypotheses

### Hypothesis 1: TX16S Channel Configuration (MOST LIKELY)
**Theory**: TX16S channels configured as unipolar (0-100%) instead of bipolar (-100% to +100%)

**How this would cause the symptom**:
- Unipolar: Stick DOWN = 0%, CENTER = 50%, UP = 100%
- CRSF would send: MIN=172 (0%), MID=992 (50%), MAX=1811 (100%)
- Our normalize_channel() would produce:
  - Stick DOWN: (172-992)/(1811-992) = -1.0 ✓
  - Stick UP: (1811-992)/(1811-992) = +1.0 ✓

Wait, that's NOT it... unipolar would make the range 0.0 to +1.0, not cause both directions to be the same.

**Alternative**: TX16S channel **direction is reversed** AND something else is wrong.

**Test needed**: Check TX16S Model Setup → Outputs page:
- Channel 1 (Roll): Min/Max/Direction setting
- Channel 2 (Pitch): Min/Max/Direction setting
- Channel 4 (Yaw): Min/Max/Direction setting
- Verify that moving stick shows values changing from NEGATIVE to POSITIVE

### Hypothesis 2: CRSF Channel Not Changing
**Theory**: The raw CRSF channel value is STUCK at one value regardless of stick position

**How this would cause the symptom**:
- If raw channel always reads 1811 (max), normalized would always be +1.0
- If raw channel always reads 172 (min), normalized would always be -1.0
- This would cause unidirectional behavior

**Why this might happen**:
- CRSF bit unpacking error in `crsf_unpack_channels()`
- TX16S not actually transmitting channel changes
- Receiver not receiving channel changes
- Channel source in TX16S set to a fixed value instead of stick

**Test needed**: Add visual debug to see raw CRSF value
- Option A: LED blink pattern based on raw value
- Option B: Motor speed proportional to raw value (not normalized)
- Option C: Use different Arduino with Serial debug capability

### Hypothesis 3: CRSF Bit Unpacking Error
**Theory**: The 11-bit channel extraction in `crsf_unpack_channels()` has a bug

**Code to verify**:
```cpp
// Channel 1 (bits 0-10 of payload)
channels[0] = ((payload[0] | payload[1] << 8)) & 0x07FF;

// Channel 2 (bits 11-21 of payload)
channels[1] = ((payload[1]>>3 | payload[2] << 5)) & 0x07FF;
```

**How this would cause the symptom**:
- If bit extraction is wrong, might always extract same bits regardless of payload
- Would need to manually verify bit math

**Test needed**: Manual calculation of expected bit positions for CRSF channels 1, 2, 4

### Hypothesis 4: TX16S Channel Source Wrong
**Theory**: TX16S channels mapped to wrong source (switch instead of stick?)

**How this would cause the symptom**:
- If channel source is a 2-position switch, would only output MIN or MAX
- Would appear unidirectional

**Test needed**: Check TX16S Model Setup → Mixer page:
- CH1 source should be "Ail" (right stick X)
- CH2 source should be "Ele" (right stick Y)
- CH4 source should be "Rud" (left stick X)

## What We've Tried (And Failed)

### ❌ Attempt 1: Invert Pitch (2024-12-29)
```cpp
float y = -g_state.input.pitch;
```
**Result**: Still unidirectional (both directions → same motion)
**Why it failed**: Problem is not polarity inversion

### ❌ Attempt 2: Invert Yaw (2024-12-29)
```cpp
float r = -g_state.input.yaw;
```
**Result**: Still unidirectional
**Why it failed**: Problem is not polarity inversion

### ❌ Attempt 3: Invert Roll (2024-12-29)
```cpp
float x = -g_state.input.roll;
```
**Result**: Still unidirectional
**Why it failed**: Problem is not polarity inversion

### ❌ Attempt 4: Remove All Inversions (2024-12-29)
```cpp
float x = g_state.input.roll;
float y = g_state.input.pitch;
float r = g_state.input.yaw;
```
**Result**: Still unidirectional (baseline test)
**Why it failed**: Confirmed inversions are not the solution

## Next Steps (MUST DO IN ORDER)

### Step 1: TX16S Configuration Audit ⏸️ WAITING FOR USER
**Action**: User to check TX16S settings
**What to check**:
1. Model Setup → Outputs → CH1, CH2, CH4:
   - Min value (should be around -100)
   - Max value (should be around +100)
   - Direction (Normal or Inverted)
2. Model Setup → Mixer → CH1, CH2, CH4:
   - Source (should be Ail, Ele, Rud respectively)
   - Weight (should be 100%)
3. Main screen → Channel monitor:
   - Move right stick UP/DOWN: Does CH2 value go from -100 to +100?
   - Move right stick LEFT/RIGHT: Does CH1 value go from -100 to +100?
   - Move left stick LEFT/RIGHT: Does CH4 value go from -100 to +100?

**If channels DON'T go negative**: TX16S configuration issue (fix in radio)
**If channels DO go negative**: CRSF parsing bug (fix in firmware)

### Step 2: Raw CRSF Value Diagnostic (If Step 1 shows TX16S is correct)
**Action**: Create visual debug to show raw CRSF channel values
**Implementation**: Motor speed varies based on raw value (not normalized)
```cpp
// Test: Map raw CRSF value directly to motor speed
uint16_t raw = crsf_parser.channels_raw[1];  // Pitch channel
// Map 172-1811 to 0-255 motor speed
int16_t test_speed = map(raw, 172, 1811, -200, 200);
actuators_set_motor(0, test_speed);
```
**Expected**: Motor speed changes as stick moves
**If speed doesn't change**: Raw value is stuck (CRSF unpacking bug or receiver issue)

### Step 3: CRSF Bit Unpacking Audit (If Step 2 shows raw value stuck)
**Action**: Manually verify CRSF channel extraction math
**Focus**: Channels 0, 1, 3 (Roll, Pitch, Yaw)
**Reference**: CRSF protocol spec (16 channels × 11 bits = 176 bits = 22 bytes)

### Step 4: Consider AlfredoCRSF Library (If Step 3 finds unfixable bug)
**Action**: Replace custom CRSF parsing with proven library
**Risk**: Unknown memory footprint, significant code rewrite
**Benefit**: Eliminates entire class of parsing bugs

## Files Involved

| File | Relevance | Lines of Interest |
|------|-----------|-------------------|
| `src/input.cpp` | CRSF parsing | 53-75 (unpack), 84-95 (normalize), 259-266 (mapping) |
| `src/mixing.cpp` | Holonomic mixing | 107-140 (mixing_update) |
| `src/actuators.cpp` | Motor control | 120-165 (AFMotor control) |
| `include/config.h` | Motor polarity | 89-92 (inversion flags) |
| `include/input.h` | CRSF constants | 26-28 (MIN/MID/MAX values) |

## Memory Usage

**Current** (with holonomic mixing restored):
- RAM: 393/2048 bytes (19.2%)
- Flash: 9880/32256 bytes (30.6%)

**Headroom**: 1655 bytes RAM, 22376 bytes Flash (plenty for library if needed)

## Decision Tree

```
Is TX16S channel monitor showing -100 to +100 range?
├─ NO → Fix TX16S configuration (outputs or mixer)
└─ YES → Is raw CRSF value changing?
    ├─ NO → CRSF unpacking bug or receiver issue
    │   └─ Verify bit extraction math
    │       ├─ Bug found → Fix bit math
    │       └─ No bug → Try AlfredoCRSF library
    └─ YES → normalize_channel() bug (unlikely, math verified)
        └─ Add clamping or different formula
```

## Priority

**CRITICAL** - Robot non-functional until resolved. Blocks all navigation testing.

---

## Resolution

**Status**: ✅ RESOLVED
**Resolution Date**: 2026-01-01
**Root Cause**: Custom CRSF parser had bit unpacking issues
**Solution**: Replaced custom CRSF parser with AlfredoCRSF library

**Changes Made**:
1. Integrated AlfredoCRSF library (sub/AlfredoCRSF)
2. Rewrote `src/input.cpp` to use `crsf.getChannel()` API
3. All 16 channels now properly bidirectional
4. Library handles CRSF protocol parsing correctly

**Verification**:
- ✅ Pitch (forward/backward) - bidirectional
- ✅ Roll (strafe left/right) - bidirectional
- ✅ Yaw (rotate left/right) - bidirectional
- ✅ All switches functional (SA, SB, SF, SH)
- ✅ Battery telemetry working

**Related Commits**:
- `201499b` - Integrate AlfredoCRSF library and fix holonomic drive issues
- `6066773` - Invert strafe direction for correct X-axis control
- `6b1eba4` - Implement drive modes and kill switch functionality

---

**Created**: 2024-12-29
**Status**: completed
**Last Updated**: 2026-01-01
**Assigned**: embedded-developer
**Blocker**: None - RESOLVED
