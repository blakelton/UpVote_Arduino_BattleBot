# Gap Analysis V3: Monitoring Strategy (LED + CRSF Telemetry)

**Date**: 2025-12-25 (Post-Monitoring Strategy Change)
**Reviewer**: Claude Sonnet 4.5
**Scope**: Targeted analysis of LED diagnostics + CRSF telemetry vs. Serial logging approach

---

## Executive Summary

**Overall Assessment**: ⭐⭐⭐⭐ (4/5 - Very Good, Minor Gaps Identified)

**Status**: The monitoring strategy change from Serial logging to LED + CRSF telemetry is **sound and well-designed**. A few minor gaps and edge cases have been identified that should be addressed before implementation.

### Quick Findings
✅ **Strengths**: 8 major improvements identified
⚠️ **Minor Gaps**: 5 issues requiring mitigation
❌ **Critical Gaps**: 0

---

## Change Summary

### What Changed
**Before**: Serial logging at 115200 baud with 500ms heartbeat (Phase 1.7)
**After**:
1. Enhanced LED diagnostics with error code blinking (Phase 1.5)
2. CRSF telemetry to TX16S display (Phase 2.5)

### Rationale
- Avoid UART conflict with CRSF receiver (420kbaud on pins 0/1)
- Enable monitoring during competition (no USB cable needed)
- Professional-grade diagnostics visible on transmitter

---

## Gap Analysis by Category

### 1. Hardware Considerations

#### GAP-M1: ELRS Receiver RX Pin Support (MEDIUM PRIORITY)
**Issue**: Not all ELRS receivers expose an RX pin for bidirectional telemetry.

**Impact**:
- If receiver lacks RX pin, telemetry won't work
- User would need to verify receiver model supports bidirectional CRSF

**Evidence in Plans**:
- Phase 2.5 mentions: "Check your receiver datasheet to identify the RX pin!"
- Risk table includes: "Receiver doesn't support RX pin" (Medium impact)

**Mitigation Status**: ⚠️ PARTIAL
- Plan acknowledges the issue
- Telemetry is optional (can be disabled via `CRSF_TELEMETRY_ENABLED`)
- **MISSING**: Pre-Phase 2 requirement to verify receiver supports bidirectional CRSF

**Recommendation**:
Add to Phase 2 Pre-Phase Requirements:
```markdown
### Hardware Verification
- [ ] **Verify ELRS receiver supports bidirectional CRSF**
  - Check receiver datasheet for "RX" or "T" pin (telemetry input)
  - Common models with telemetry: EP1/EP2 (yes), SuperP (yes), Nano RX (some variants)
  - If no RX pin: Set `CRSF_TELEMETRY_ENABLED` to 0 (LED-only diagnostics)
```

---

#### GAP-M2: Battery Voltage Monitoring Hardware (LOW PRIORITY)
**Issue**: Telemetry can send battery voltage, but requires voltage divider circuit on A0.

**Impact**:
- Without voltage divider, telemetry sends placeholder (7.4V nominal)
- User may expect actual voltage monitoring without realizing hardware needed

**Evidence in Plans**:
- Phase 2.5 mentions: "Optional: Battery voltage monitor circuit (A0 with voltage divider)"
- Code has placeholder: `voltage_mv = 7400; // Placeholder`

**Mitigation Status**: ✅ ACCEPTABLE
- Plan clearly states it's optional
- Placeholder value is reasonable
- **NICE TO HAVE**: Add voltage divider schematic to documentation

**Recommendation**:
Add to `docs/hardware/` (Phase 7 documentation):
```markdown
### Optional: Battery Voltage Monitor
- 2S LiPo: 6.4V - 8.4V range
- Arduino ADC: 0-5V max (8.4V exceeds this!)
- Required: Voltage divider (e.g., 10kΩ + 10kΩ) to scale 8.4V → 4.2V
- Connect to A0, calibrate `BATTERY_SCALE_FACTOR` in config.h
```

---

#### GAP-M3: LED Visibility in Bright Light (LOW PRIORITY)
**Issue**: Built-in LED (pin 13) may not be visible in bright sunlight or if Arduino is enclosed.

**Impact**:
- Difficult to see LED patterns during outdoor testing
- May miss error codes if LED obscured by hardware

**Evidence in Plans**:
- Phase 1.5 testing: "LED visible from 3 meters in bright light"
- Acceptance: "LED visible and not obscured by hardware"

**Mitigation Status**: ⚠️ NEEDS ATTENTION
- Plan includes visibility test
- **MISSING**: Recommendation for external LED if needed

**Recommendation**:
Add to Phase 1.5 Open Questions:
```markdown
- [ ] If built-in LED not visible: Add external LED on pin 13 (220Ω resistor)
  - Recommendation: High-brightness LED (red for errors, green for normal)
  - Mount externally on bot chassis for competition visibility
```

---

### 2. Software/Firmware Considerations

#### GAP-M4: LED Blink Code State Machine Complexity (MEDIUM PRIORITY)
**Issue**: Error code blinking state machine (`diagnostics_blink_error_code`) is complex with static state variables.

**Impact**:
- Static variables (`error_blink_count`, `error_blink_phase`) persist across calls
- If error code CHANGES during blinking sequence, behavior undefined
- May blink wrong count if error changes mid-sequence

**Evidence in Plans**:
Phase 1.5 code:
```cpp
static uint8_t error_blink_count = 0;
static uint8_t error_blink_phase = 0;

void diagnostics_blink_error_code(SystemError error) {
  uint8_t blink_count = (uint8_t)error;
  // Uses static state...
}
```

**Mitigation Status**: ❌ NEEDS FIX
- No handling for error code change during blink sequence
- Could confuse user (starts blinking 2, error changes to 3, blinks mixed pattern)

**Recommendation**:
Add error change detection:
```cpp
static SystemError last_error = ERR_NONE;

void diagnostics_blink_error_code(SystemError error) {
  uint8_t blink_count = (uint8_t)error;

  // Reset state if error changed
  if (error != last_error) {
    error_blink_phase = 0;
    error_blink_count = 0;
    last_error = error;
  }

  // ... rest of state machine ...
}
```

---

#### GAP-M5: CRSF Telemetry CRC Function Dependency (MEDIUM PRIORITY)
**Issue**: Phase 2.5 telemetry uses `crsf_crc8()` function, but it's defined in Phase 2.1.

**Impact**:
- If user implements Phase 2.5 without careful Phase 2.1 integration, CRC function missing
- Code won't compile

**Evidence in Plans**:
- Phase 2.5 code: `uint8_t crc = crsf_crc8(&packet[2], idx - 2);`
- Phase 2.1 defines the CRC function
- Dependencies list: "Requires: Phase 2.4 complete" but doesn't mention CRC function

**Mitigation Status**: ⚠️ CLARIFICATION NEEDED
- Dependency stated but not explicit about CRC function

**Recommendation**:
Update Phase 2.5 Dependencies:
```markdown
#### Dependencies
- Requires: Phase 2.1 complete (CRSF CRC-8 function implemented)
- Requires: Phase 2.4 complete (CRSF RX working)
- Requires: Diagnostics module with `get_free_ram()` (Phase 1.5)
```

---

#### GAP-M6: Telemetry Serial.write() Timing Impact (LOW PRIORITY)
**Issue**: `Serial.write(packet, idx)` in telemetry is blocking; could impact loop timing.

**Impact**:
- 16 bytes at 420kbaud = ~0.3ms transmission time
- Should be acceptable (loop budget is 10ms)
- Risk if called too frequently or if buffer is full

**Evidence in Plans**:
- Phase 2.5 risk table: "Serial.write() blocks loop" with mitigation "Limit to 1 Hz"
- 1 Hz update rate = 0.3ms every 1000ms = negligible impact

**Mitigation Status**: ✅ ADDRESSED
- 1 Hz rate is conservative
- Risk acknowledged in plan

**Recommendation**: None needed, already mitigated.

---

#### GAP-M7: Free RAM Calculation Edge Cases (LOW PRIORITY)
**Issue**: `diagnostics_get_free_ram()` uses stack pointer trick; may be inaccurate if heap fragmented.

**Impact**:
- Reported RAM may differ from actual free RAM
- On Arduino UNO with static allocation only, should be accurate
- Edge case: If any dynamic allocation occurs (shouldn't happen), value misleading

**Evidence in Plans**:
Phase 1.5 code:
```cpp
int diagnostics_get_free_ram() {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
```

**Mitigation Status**: ✅ ACCEPTABLE
- Static allocation only (NFR-5: "No dynamic memory allocation")
- Free RAM calculation should be accurate
- Testing includes: "Compare to build output for validation"

**Recommendation**: None needed, already sound.

---

### 3. Protocol/Communication Considerations

#### GAP-M8: CRSF Telemetry Packet Timing with RC Channels (MEDIUM PRIORITY)
**Issue**: Both RC Channels (RX) and Telemetry (TX) use same Serial object at 420kbaud.

**Impact**:
- Potential timing collision: Arduino reading Serial (RX) while sending telemetry (TX)?
- Hardware UART should handle full-duplex, but software state may conflict

**Evidence in Plans**:
- Phase 2.5 risk: "Telemetry interferes with RC Channels" (Low impact, "Test extensively")
- No explicit verification that Serial object handles simultaneous RX/TX

**Mitigation Status**: ⚠️ NEEDS TESTING
- Risk acknowledged but marked "Low"
- **CRITICAL**: This needs empirical testing
- Arduino hardware UART *should* handle full-duplex, but verify

**Recommendation**:
Update Phase 2.5 Testing Strategy:
```markdown
- **Simultaneous RX/TX test**:
  - Send telemetry while receiving RC Channels packets
  - Use logic analyzer to verify no packet corruption
  - Stress test: High RC packet rate (150Hz) + telemetry (1Hz)
  - Verify no RC Channels packets dropped during telemetry transmission
```

---

#### GAP-M9: TX16S Telemetry Sensor Discovery Failure (LOW PRIORITY)
**Issue**: TX16S may not auto-discover telemetry sensor if timing or packet format incorrect.

**Impact**:
- User frustration if "Discover sensors" doesn't find battery sensor
- Difficult to debug without logic analyzer

**Evidence in Plans**:
- Phase 2.5 risk: "TX16S doesn't detect sensor" (Low impact)
- Mitigation: "Use 'Discover sensors' menu item"

**Mitigation Status**: ⚠️ TROUBLESHOOTING NEEDED
- Risk acknowledged
- **MISSING**: Troubleshooting steps if discovery fails

**Recommendation**:
Add to Phase 2.5 Troubleshooting section:
```markdown
### Troubleshooting: TX16S Doesn't Detect Sensor

1. **Verify telemetry packets sent**: Use logic analyzer on pin 1
2. **Check CRC**: Verify CRC-8 calculation matches CRSF spec
3. **Verify packet format**: Battery sensor (0x08) frame structure
4. **Check wiring**: Arduino TX (pin 1) → Receiver RX pin
5. **Update rate**: Ensure 1 Hz is sufficient (TX16S may need faster initial discovery)
6. **Try manual sensor**: Create custom sensor on TX16S if auto-discovery fails
```

---

### 4. Operational/Usability Considerations

#### GAP-M10: LED Error Code Memorization (LOW PRIORITY)
**Issue**: User must memorize LED error codes (1=loop overrun, 2=watchdog, 3=CRSF timeout, etc.).

**Impact**:
- During competition, user may not remember what "3 blinks" means
- Error code lookup requires consulting documentation

**Evidence in Plans**:
- Phase 1.5 has LED pattern reference table
- Phase 7 includes operator manual deliverable

**Mitigation Status**: ✅ ACCEPTABLE
- Reference table documented
- Operator manual will include LED code guide
- **NICE TO HAVE**: Quick reference card for pit area

**Recommendation**:
Add to Phase 7 operator manual:
```markdown
### Quick Reference: LED Error Codes (Laminated Card for Pit)
| Blinks | Error | Meaning | Action |
|--------|-------|---------|--------|
| 1 | Loop Overrun | Code too slow | Check for blocking code |
| 2 | Watchdog Reset | System lockup | Check for infinite loops |
| 3 | CRSF Timeout | Link loss | Check receiver/TX16S |
| 4 | CRSF CRC Error | Packet corruption | Check wiring/interference |
```

---

#### GAP-M11: No Debug Output During Phase 1 Bench Testing (MEDIUM PRIORITY)
**Issue**: Phase 1 has NO Serial output (removed Phase 1.7), only LED patterns.

**Impact**:
- Difficult to debug Phase 1 issues without Serial output
- Can't see detailed state (loop timing, exact error codes, etc.)
- Reduced troubleshooting capability during early development

**Evidence in Plans**:
- Phase 1 Open Questions: "Serial debug enabled in Phase 1? (Recommend: disabled to save resources)"
- This was changed from "recommended disabled" to "actually disabled"

**Mitigation Status**: ⚠️ NEEDS OPTION
- No Serial output at all in Phase 1
- LED patterns are good but limited detail
- **ISSUE**: User explicitly wanted monitoring "early on to help me with troubleshooting"

**Recommendation**:
**CRITICAL MITIGATION**: Add conditional Serial debug back to Phase 1:
```cpp
// In config.h
#define DEBUG_SERIAL_PHASE1  1  // Set to 1 for Phase 1 bench testing
                                 // Set to 0 before Phase 2 (UART conflict)

#if DEBUG_SERIAL_PHASE1
  // Phase 1 ONLY: Serial debug at 115200
  // WARNING: Must disable before Phase 2!
#endif
```

Add compile-time safety check in Phase 2:
```cpp
// In src/input.cpp (Phase 2)
#if DEBUG_SERIAL_PHASE1
  #error "CRITICAL: Set DEBUG_SERIAL_PHASE1 to 0 in Phase 2! UART conflict with CRSF."
#endif
```

This gives user troubleshooting capability in Phase 1, with hard error if they forget to disable it.

---

### 5. Testing Considerations

#### GAP-M12: No Automated LED Pattern Testing (LOW PRIORITY)
**Issue**: LED pattern verification is manual ("Visual test: Verify all 4 patterns").

**Impact**:
- Human error in counting blinks
- Difficult to regression test LED patterns after changes
- No automated validation

**Evidence in Plans**:
- Phase 1.5 testing: "Visual test", "Error code test: count LED blinks"
- All LED testing is manual

**Mitigation Status**: ✅ ACCEPTABLE
- Automated LED testing is difficult on embedded systems
- Manual testing is standard practice for LEDs
- **OPTIONAL**: Could use photodiode + scope for automated testing (overkill)

**Recommendation**: None needed (manual testing acceptable).

---

### 6. Memory/Performance Considerations

#### GAP-M13: Telemetry Memory Overhead Verification (LOW PRIORITY)
**Issue**: Phase 2.5 claims "~20 bytes" overhead, but needs empirical verification.

**Impact**:
- If overhead exceeds estimate, may breach Phase 2 budget (768 bytes)
- Stack-allocated packet buffer (16 bytes) may cause stack overflow if many nested calls

**Evidence in Plans**:
- Memory Impact section: "Telemetry packet buffer: 16 bytes (stack allocated, no permanent cost)"
- Phase 2 budget: 768 bytes total

**Mitigation Status**: ⚠️ NEEDS VERIFICATION
- Estimate seems reasonable
- **REQUIRED**: Build-time verification with `-Wl,--print-memory-usage`

**Recommendation**:
Add to Phase 2.5 Acceptance Criteria:
```markdown
- [ ] Build output confirms SRAM increase ≤30 bytes vs. Phase 2.4
- [ ] Stack usage measured (if tools available)
- [ ] Total Phase 2 SRAM still <768 bytes with telemetry enabled
```

---

## Summary of Gaps

### Critical Gaps (Must Fix Before Implementation)
**None identified.** ✅

### High Priority Gaps (Should Fix Before Implementation)
**None identified.** ✅

### Medium Priority Gaps (Address During Implementation)
1. **GAP-M1**: Verify ELRS receiver RX pin support (add to Phase 2 pre-reqs)
2. **GAP-M4**: LED error code state machine - handle error changes
3. **GAP-M5**: Clarify CRSF CRC function dependency in Phase 2.5
4. **GAP-M8**: Test CRSF telemetry timing with RC Channels (simultaneous RX/TX)
5. **GAP-M11**: **CRITICAL** - Add conditional Serial debug option for Phase 1 troubleshooting

### Low Priority Gaps (Nice to Have)
6. **GAP-M2**: Document battery voltage divider schematic
7. **GAP-M3**: External LED recommendation if built-in not visible
8. **GAP-M9**: TX16S discovery troubleshooting guide
9. **GAP-M10**: Quick reference card for LED error codes
10. **GAP-M13**: Verify telemetry memory overhead empirically

---

## Comparison: Serial Logging vs. LED + Telemetry

| Aspect | Serial Logging (V2) | LED + Telemetry (V3) | Winner |
|--------|---------------------|----------------------|--------|
| UART conflict | ❌ Conflicts with CRSF | ✅ Uses CRSF bidirectional | V3 |
| Competition use | ❌ Needs USB cable | ✅ Wireless telemetry | V3 |
| Phase 1 debug | ✅ Detailed output | ⚠️ LED only (limited) | V2 |
| Visibility | ⚠️ Needs computer | ✅ TX16S + LED | V3 |
| Complexity | ✅ Simple (Serial.print) | ⚠️ CRSF packet building | V2 |
| Memory overhead | ✅ ~50 bytes (strings in PROGMEM) | ✅ ~20 bytes | V2 |
| Real-time monitoring | ❌ Tethered to computer | ✅ TX16S in hand | V3 |
| Error codes | ✅ Text readable | ⚠️ Need to memorize blinks | V2 |
| Professionalism | ⚠️ Hobbyist (USB cable) | ✅ Pro (telemetry) | V3 |

**Overall Winner**: LED + Telemetry (V3) - **7 wins vs. 3 wins**

**Key Trade-off**: Phase 1 detailed debugging is reduced, but overall system is more professional and competition-ready.

**Critical Recommendation**: Add back **conditional Serial debug for Phase 1 only** (GAP-M11) to preserve troubleshooting capability.

---

## Recommended Plan Updates

### 1. Phase 1.5 - Enhanced LED Diagnostics
**Add to `diagnostics_blink_error_code()`**:
```cpp
static SystemError last_error = ERR_NONE;

void diagnostics_blink_error_code(SystemError error) {
  uint8_t blink_count = (uint8_t)error;

  // Reset state if error changed (FIX for GAP-M4)
  if (error != last_error) {
    error_blink_phase = 0;
    error_blink_count = 0;
    last_blink = millis();
    last_error = error;
  }

  // ... rest of state machine ...
}
```

**Add to Open Questions**:
```markdown
- [ ] If built-in LED not visible: Add external high-brightness LED on pin 13 (220Ω resistor)
```

### 2. Phase 1 - Add Conditional Debug Serial (NEW)
**Add to `include/config.h`**:
```cpp
// Debug Serial for Phase 1 ONLY
// ⚠️ DISABLE THIS BEFORE PHASE 2! (UART conflict with CRSF)
#define DEBUG_SERIAL_PHASE1  1  // 1=enabled (bench testing), 0=disabled (flight/Phase2+)

#if DEBUG_SERIAL_PHASE1
  #define DEBUG_BAUD_RATE      115200
  #define DEBUG_HEARTBEAT_MS   500
#endif
```

**Add to `src/diagnostics.cpp`**:
```cpp
#if DEBUG_SERIAL_PHASE1
void diagnostics_print_debug() {
  extern RuntimeState g_state;
  uint32_t now = millis();
  static uint32_t last_debug = 0;

  if (now - last_debug >= DEBUG_HEARTBEAT_MS) {
    last_debug = now;
    Serial.print(F("Loop:"));
    Serial.print(g_state.loop_count);
    Serial.print(F(" Err:"));
    Serial.print((int)safety_get_error());
    Serial.print(F(" RAM:"));
    Serial.print(diagnostics_get_free_ram());
    Serial.println(F("B"));
  }
}
#endif
```

**Add to Phase 2 Pre-Phase Requirements**:
```markdown
### ⚠️ CRITICAL: Disable Phase 1 Debug Serial
- [ ] **Set `DEBUG_SERIAL_PHASE1` to `0` in `include/config.h`**
- [ ] Rebuild and verify UART freed for CRSF
```

**Add to `src/input.cpp` (Phase 2)**:
```cpp
#if DEBUG_SERIAL_PHASE1
  #error "CRITICAL: Set DEBUG_SERIAL_PHASE1 to 0 before Phase 2! UART conflict with CRSF."
#endif
```

### 3. Phase 2.5 - CRSF Telemetry
**Update Dependencies**:
```markdown
#### Dependencies
- Requires: **Phase 2.1 complete (CRSF CRC-8 function implemented)** ⭐ UPDATED
- Requires: Phase 2.4 complete (CRSF RX working)
- Requires: Diagnostics module with `get_free_ram()` (Phase 1.5)
- Optional: Battery voltage monitor circuit (A0 with voltage divider)
```

**Add to Testing Strategy**:
```markdown
- **Simultaneous RX/TX stress test**: Send telemetry (1 Hz) while receiving RC Channels (150 Hz)
  - Use logic analyzer to verify no packet loss or corruption
  - Verify RC Channels latency unchanged (<200ms link check)
```

**Add Troubleshooting Section**:
```markdown
#### Troubleshooting: TX16S Doesn't Detect Sensor
1. Verify telemetry packets sent (logic analyzer on pin 1)
2. Check CRC calculation (compare to CRSF spec test vectors)
3. Verify frame structure (Battery Sensor 0x08 format)
4. Check wiring: Arduino TX (pin 1) → Receiver RX pin
5. Try faster update rate temporarily (100ms instead of 1000ms) for discovery
6. Manually create sensor on TX16S if auto-discovery fails
```

**Add to Pre-Phase Requirements**:
```markdown
### Hardware Verification
- [ ] **Verify ELRS receiver supports bidirectional CRSF (has RX pin)**
  - Check receiver datasheet/pinout diagram
  - Common models: EP1/EP2 (yes), SuperP (yes), Nano RX (check variant)
  - If no RX pin: Set `CRSF_TELEMETRY_ENABLED` to 0 (LED-only mode)
```

### 4. Phase 7 - Documentation
**Add to Operator Manual**:
```markdown
### LED Error Code Quick Reference
Print and laminate for pit area:

| Blinks | Error | Action |
|--------|-------|--------|
| 1 | Loop Overrun | Report to developer |
| 2 | Watchdog Reset | Power cycle, check for lockup |
| 3 | CRSF Timeout | Check TX16S power, receiver connection |
| 4 | CRC Error | Check wiring, RF interference |

**LED Patterns**:
- Slow blink (1 Hz): SAFE (ready to drive)
- Fast blink (5 Hz): ARMED (weapon active!)
- Solid ON: FAILSAFE (link loss or kill switch)
```

**Add Battery Voltage Monitor Schematic**:
```markdown
### Optional: Battery Voltage Monitor Circuit
```
       2S LiPo (6.4V - 8.4V)
            |
         [10kΩ]  ← R1
            |-----> To Arduino A0 (0-4.2V)
         [10kΩ]  ← R2
            |
           GND

Voltage divider: Vout = Vin × (R2 / (R1 + R2))
Max input: 8.4V → Output: 4.2V (safe for Arduino ADC)
Calibrate BATTERY_SCALE_FACTOR = 4.2 / 1023 / 2 = 0.00205
```
```

---

## Final Assessment

### Overall Grade: **A- (Very Good)**

**Strengths** (8):
1. ✅ Eliminates UART conflict completely
2. ✅ Professional telemetry system
3. ✅ Competition-ready (no USB cable needed)
4. ✅ LED diagnostics work everywhere
5. ✅ Minimal memory overhead (~20 bytes)
6. ✅ CRSF bidirectional by design
7. ✅ Real-time TX16S display
8. ✅ Multiple monitoring layers (LED + telemetry)

**Weaknesses** (5 medium, 5 low):
1. ⚠️ **GAP-M11 (CRITICAL)**: Phase 1 troubleshooting reduced without Serial option
2. ⚠️ GAP-M1: Receiver RX pin support not verified upfront
3. ⚠️ GAP-M4: LED error blink state machine edge case
4. ⚠️ GAP-M8: Simultaneous RX/TX testing needed
5. ⚠️ GAP-M5: CRC dependency not explicit

**Recommendation**: **APPROVE WITH MINOR UPDATES**

The monitoring strategy change is a significant improvement over Serial logging. The most critical gap (GAP-M11) can be resolved by adding conditional Serial debug for Phase 1 only, giving the user troubleshooting capability when needed while avoiding UART conflicts in later phases.

**Action Items Before Implementation**:
1. Add conditional `DEBUG_SERIAL_PHASE1` flag (GAP-M11) ← **DO THIS FIRST**
2. Update Phase 2 Pre-Phase Requirements to verify receiver RX pin (GAP-M1)
3. Fix LED blink error code state machine (GAP-M4)
4. Clarify Phase 2.5 dependencies (GAP-M5)

---

**Status**: **APPROVED FOR IMPLEMENTATION** (with recommended updates)

**Confidence Level**: **HIGH** 🚀

The LED + CRSF telemetry approach is the right design for a competition battlebot. With the recommended Phase 1 Serial debug option, the user gets best of both worlds: detailed troubleshooting early, professional telemetry later.

---

**Reviewer**: Claude Sonnet 4.5
**Date**: 2025-12-25
**Status**: **READY TO PROCEED** (after addressing GAP-M11)
