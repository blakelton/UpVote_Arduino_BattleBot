# Feature Plan: Replace Custom CRSF Parser with AlfredoCRSF Library

## Overview
Replace our custom CRSF parsing implementation with the proven AlfredoCRSF library to eliminate the float normalization bug that's causing unidirectional control issues.

## Problem Statement

### Current Issues
1. **Custom CRSF parser has normalization bug** - Float division producing incorrect bidirectional values
2. **Integer `map()` works, but float normalize/deadband doesn't** - Proven through diagnostics
3. **Complex debugging without serial output** - CRSF occupies the serial port
4. **Reinventing the wheel** - AlfredoCRSF is battle-tested and maintained

### Root Cause Analysis
Through extensive diagnostic testing, we've confirmed:
- ✅ Raw CRSF values ARE changing correctly (172 → 992 → 1811)
- ✅ `map()` integer math works perfectly (first diagnostic test)
- ✅ AFMotor library preserves sign correctly
- ✅ Holonomic mixing equations are correct
- ❌ Float normalization pipeline has a bug (normalize_channel + apply_deadband)
- ❌ Even manual float normalization fails identically

**Conclusion**: There's a subtle bug in our float-based normalization that we haven't found. Rather than continue debugging, switch to proven library.

## Current State

### What We Have
- **Custom CRSF parser** in `src/input.cpp`:
  - `crsf_unpack_channels()` - Bit unpacking (seems to work)
  - `normalize_channel()` - Float normalization (BUGGY)
  - `apply_deadband()` - Deadband with rescaling (possibly buggy)
  - CRC validation
  - Frame synchronization state machine

- **Working integer-based holonomic mixing** in `src/mixing.cpp`:
  - Uses `map()` directly on raw_channels
  - Bypasses float normalization entirely
  - Integer arithmetic throughout

### What's Broken
- Float-based normalization produces unidirectional values
- Cannot use expo curves (requires float math)
- Cannot use drive modes properly (expo is broken)

## Requirements

### FR-1: Install AlfredoCRSF Library
- Add as PlatformIO library dependency
- Submodule already added: `sub/AlfredoCRSF`
- Configure library include path

### FR-2: Replace CRSF Parsing Code
- Remove custom parser from `src/input.cpp`
- Replace with AlfredoCRSF API calls
- Maintain same channel mapping (CH1=roll, CH2=pitch, CH4=yaw)

### FR-3: Integrate with Existing Architecture
- Populate `g_state.input` fields from library
- Maintain link status checking via `isLinkUp()`
- Preserve switch decoding logic
- Keep weapon throttle mapping

### FR-4: Restore Float-Based Mixing (Optional)
- Once AlfredoCRSF provides clean values, can restore float mixing
- Re-enable expo curves
- Re-enable drive mode support
- Add deadband back (but test thoroughly)

## Implementation Plan

### Phase 1: Library Integration

#### Step 1.1: Configure PlatformIO
**File**: `platformio.ini`
**Changes**:
```ini
lib_deps =
    adafruit/Adafruit Motor Shield library@^1.0.1
    file://sub/AlfredoCRSF  ; Local submodule
```

#### Step 1.2: Update Input Module Header
**File**: `include/input.h`
**Changes**:
```cpp
#include <AlfredoCRSF.h>

// Remove custom CRSF constants (library handles these)
// Keep only:
#define INPUT_DEADBAND 0.05f

// Expose AlfredoCRSF instance
extern AlfredoCRSF crsf;

void input_init();
void input_update();
```

#### Step 1.3: Rewrite Input Implementation
**File**: `src/input.cpp`
**Changes**:
```cpp
#include <AlfredoCRSF.h>

// Global CRSF instance
AlfredoCRSF crsf;

void input_init() {
  // Initialize CRSF library on Hardware Serial
  Serial.begin(CRSF_BAUDRATE);  // 420000 baud
  crsf.begin(Serial);
}

void input_update() {
  // Update CRSF library (reads serial, parses packets)
  crsf.update();

  // Get link status
  g_state.input.link_ok = crsf.isLinkUp();
  if (g_state.input.link_ok) {
    g_state.input.last_packet_ms = millis();
  }

  // Read channels (getChannel returns microseconds)
  // Channel mapping: 1-based indexing
  uint16_t ch1_us = crsf.getChannel(1);  // Roll (right stick X)
  uint16_t ch2_us = crsf.getChannel(2);  // Pitch (right stick Y)
  uint16_t ch4_us = crsf.getChannel(4);  // Yaw (left stick X)
  uint16_t ch5_us = crsf.getChannel(5);  // Weapon slider
  uint16_t ch6_us = crsf.getChannel(6);  // Arm switch
  uint16_t ch9_us = crsf.getChannel(9);  // Kill switch

  // Store raw values for integer mixing (convert us to 11-bit)
  g_state.input.raw_channels[0] = map(ch1_us, 988, 2012, 172, 1811);
  g_state.input.raw_channels[1] = map(ch2_us, 988, 2012, 172, 1811);
  g_state.input.raw_channels[3] = map(ch4_us, 988, 2012, 172, 1811);
  g_state.input.raw_channels[4] = map(ch5_us, 988, 2012, 172, 1811);

  // Decode switches (3-position: <1200us=low, 1200-1800=mid, >1800=high)
  g_state.input.arm_switch = (ch6_us > 1800);
  g_state.input.kill_switch = (ch9_us > 1800);

  // Optional: Restore float normalization if needed
  // g_state.input.roll = map_to_float(ch1_us, 988, 2012, -1.0f, 1.0f);
  // g_state.input.pitch = map_to_float(ch2_us, 988, 2012, -1.0f, 1.0f);
  // g_state.input.yaw = map_to_float(ch4_us, 988, 2012, -1.0f, 1.0f);
}
```

### Phase 2: Update Mixing Module

#### Step 2.1: Keep Integer Mixing Initially
**File**: `src/mixing.cpp`
- Keep current integer-based mixing (proven to work)
- Continue using `g_state.input.raw_channels[]`
- Verify everything works before attempting float mixing

#### Step 2.2: (Optional) Restore Float Mixing
Once library integration is confirmed working:
```cpp
void mixing_update() {
  // Get float values directly from microseconds
  float x = map_to_float(crsf.getChannel(1), 988, 2012, -1.0f, 1.0f);
  float y = map_to_float(crsf.getChannel(2), 988, 2012, -1.0f, 1.0f);
  float r = map_to_float(crsf.getChannel(4), 988, 2012, -1.0f, 1.0f);

  // Apply deadband
  x = apply_deadband(x, INPUT_DEADBAND);
  y = apply_deadband(y, INPUT_DEADBAND);
  r = apply_deadband(r, INPUT_DEADBAND);

  // Apply expo
  x = apply_expo(x, g_mode_params.expo);
  y = apply_expo(y, g_mode_params.expo);
  r = apply_expo(r * ROTATION_SCALE, g_mode_params.expo);

  // ... rest of mixing
}
```

### Phase 3: Testing

#### Test 3.1: Verify Library Receives Data
- Upload code
- Check that `isLinkUp()` returns true
- Verify `getChannel()` returns valid microsecond values

#### Test 3.2: Verify Integer Mixing Still Works
- Test all three axes (forward/back, strafe, rotate)
- Confirm bidirectional control
- Check that motors stop at stick center

#### Test 3.3: Test Safety Systems
- Re-enable arming state check
- Verify link-loss failsafe
- Test emergency stop

#### Test 3.4: (Optional) Test Float Mixing
If restoring float-based mixing:
- Verify expo curves feel smooth
- Test drive mode switching
- Confirm deadband works correctly

## Files to Modify

| File | Type | Changes |
|------|------|---------|
| `platformio.ini` | Config | Add AlfredoCRSF to lib_deps |
| `include/input.h` | Header | Include AlfredoCRSF, expose instance |
| `src/input.cpp` | Source | Replace entire CRSF parser with library calls |
| `src/mixing.cpp` | Source | (Optional) Restore float mixing after library works |

## Files to Remove (Eventually)

These custom CRSF functions can be deleted once library is working:
- `crsf_unpack_channels()` (line 53-75 in input.cpp)
- `normalize_channel()` (line 84-95 in input.cpp)
- `apply_deadband()` (line 101-114 in input.cpp) - Keep if restoring float mixing
- CRSF constants in input.h (library defines these)
- Frame synchronization state machine
- CRC validation code

## Memory Impact

**Current Usage**:
- RAM: 420 bytes (20.5%)
- Flash: 9360 bytes (29.0%)

**Expected Change**:
- +200-400 bytes Flash (library code)
- +50-100 bytes RAM (library instance)
- -200 bytes Flash (removed custom parser)
- **Net**: +200 Flash, +75 RAM (estimated)

**Final Estimate**:
- RAM: ~495/2048 (24%) - Well within Phase 7 budget of 1800 bytes
- Flash: ~9560/32256 (29.6%) - Plenty of headroom

## Success Criteria

### SC-1: Library Integration
- ✅ Code compiles without errors
- ✅ `isLinkUp()` returns true when TX16S is on
- ✅ `getChannel()` returns values in 988-2012µs range

### SC-2: Bidirectional Control (INTEGER MIXING)
- ✅ Right stick FORWARD → bot moves forward
- ✅ Right stick BACKWARD → bot moves backward
- ✅ Right stick LEFT → bot strafes left
- ✅ Right stick RIGHT → bot strafes right
- ✅ Left stick LEFT → bot rotates left
- ✅ Left stick RIGHT → bot rotates right

### SC-3: Safety Compliance
- ✅ Motors stop when link lost
- ✅ Arming state machine works
- ✅ Kill switch functional

### SC-4: (Optional) Float Mixing Restored
If attempting float mixing:
- ✅ Expo curves functional
- ✅ Drive mode switching works
- ✅ Deadband prevents drift

## Risks & Mitigations

### Risk 1: Library Doesn't Support Arduino UNO
**Likelihood**: Low (library claims Arduino compatibility)
**Impact**: High (can't use library)
**Mitigation**:
- Test compilation first before removing old code
- Keep old parser in git history for rollback
- Library is simple C++, likely works on AVR

### Risk 2: Memory Overflow
**Likelihood**: Very Low (we have 1623 bytes RAM free)
**Impact**: Medium (need to optimize)
**Mitigation**:
- Monitor memory usage during compilation
- Library is designed for embedded, should be efficient

### Risk 3: Different Channel Value Range
**Likelihood**: Medium (library returns µs, not 11-bit)
**Impact**: Low (easy to convert)
**Mitigation**:
- Use `map()` to convert µs → 11-bit for integer mixing
- Or use `map_to_float()` for float mixing

### Risk 4: Breaking Existing Functionality
**Likelihood**: Medium (significant code changes)
**Impact**: High (robot non-functional)
**Mitigation**:
- Keep integer mixing working first (proven approach)
- Test incrementally (library → integer mixing → float mixing)
- Git branch for safety

## Rollback Plan

If library integration fails:
1. `git revert` to previous commit
2. Continue using custom parser with integer mixing
3. Investigate custom parser bug more deeply
4. Consider alternative libraries (CRSFforArduino)

## Timeline Estimate

| Phase | Duration | Dependencies |
|-------|----------|--------------|
| Library setup | 15 min | Submodule added ✓ |
| Rewrite input.cpp | 30 min | Library docs |
| Test & debug | 45 min | Hardware |
| (Optional) Float mixing | 30 min | Integer mixing works |
| **Total** | **~2 hours** | |

## Dependencies

### Prerequisites
- ✅ AlfredoCRSF submodule added
- ✅ Integer-based mixing proven to work with raw values
- ✅ AFMotor library integration complete
- ✅ TX16S configuration correct (channels bipolar)

### Blockers
- None - all prerequisites met

## Follow-Up Work

After successful integration:
1. **Remove old CRSF parser code** - Clean up input.cpp
2. **Restore expo curves** - Add float mixing back
3. **Add drive mode switching** - Restore mode support
4. **Update documentation** - Reflect library usage
5. **Update troubleshooting log** - Document findings
6. **Commit submodule** - Push .gitmodules changes

## Notes

### Why This Will Work
1. **Library is proven** - Used by FPV community extensively
2. **Simpler API** - Less code = fewer bugs
3. **Integer mixing already works** - We know the approach is sound
4. **Low risk** - Can rollback easily if issues

### Confidence Level
**HIGH** - We've isolated the problem to float normalization. AlfredoCRSF provides clean microsecond values, and we know `map()` works perfectly. This is a straightforward replacement with high probability of success.

---

**Status**: ready
**Created**: 2024-12-29
**Author**: orchestrator
**Phase**: 7 (AFMotor Integration Cleanup → CRSF Library Integration)
**Priority**: HIGH - Blocks holonomic navigation functionality
**Dependencies**: Integer mixing working, AlfredoCRSF submodule added
