# Component Analysis: Input Module (CRSF Protocol)

**Generated**: 2025-12-26
**Location**: `include/input.h`, `src/input.cpp`
**Domain**: Embedded C/C++
**Lines of Code**: 489 (50 header + 439 implementation)
**Files Analyzed**: 2

## Executive Summary

**Overall Grade**: B+
**Critical Issues**: 0
**High Issues**: 1
**Technical Debt Score**: Medium

The Input module implements a complete CRSF protocol parser with CRC validation, bit unpacking, telemetry, and state machine synchronization. This is the most complex component in the codebase. The implementation is generally solid but has some complexity hotspots and potential for refactoring. The use of PROGMEM for the CRC table is excellent RAM optimization.

## Complexity Metrics

| Metric | Value | Threshold | Status |
|--------|-------|-----------|--------|
| Max Cyclomatic Complexity | 15 | 10 | **FAIL** |
| Max Cognitive Complexity | 18 | 15 | **FAIL** |
| Max Function Length | 147 lines | 50 | **FAIL** |
| Max Nesting Depth | 5 | 4 | **FAIL** |
| Average CC | 5.2 | 5 | WARN |

### Complexity Hotspots

| Function | Lines | CC | Cognitive | Nesting | Status |
|----------|-------|-----|-----------|---------|--------|
| crsf_process_byte() | 147 | 15 | 18 | 5 | **CRITICAL** |
| input_update() | 25 | 2 | 3 | 2 | OK |
| input_update_telemetry() | 8 | 2 | 2 | 2 | OK |
| crsf_send_battery_telemetry() | 44 | 3 | 4 | 2 | OK |
| crsf_unpack_channels() | 23 | 1 | 1 | 0 | OK |
| crsf_crc8() | 6 | 2 | 2 | 1 | OK |
| normalize_channel() | 12 | 3 | 2 | 2 | OK |
| apply_deadband() | 14 | 4 | 4 | 3 | OK |
| decode_3pos_switch() | 9 | 3 | 2 | 2 | OK |

**PRIMARY CONCERN**: `crsf_process_byte()` has CC=15, cognitive complexity=18, nesting depth=5, and 147 lines. This exceeds all thresholds and should be refactored.

## DRY Analysis

### Code Duplication

| Severity | Count | Description |
|----------|-------|-------------|
| CRITICAL | 0 | No critical duplication |
| HIGH | 1 | Switch debounce pattern (duplicated in weapon.cpp) |
| MEDIUM | 2 | Input zeroing pattern appears twice |
| LOW | 3 | Repeated constrain/clamp patterns |

### Duplication Details

| Location 1 | Location 2 | Lines | Issue |
|------------|------------|-------|-------|
| input.cpp:419-427 | input.cpp:166-173 | 8 | Input reset to zero values |
| input.cpp:281-297 | Multiple places | ~3-5 | normalize + apply_deadband pattern |

### Magic Values

| Location | Value | Issue | Suggested Constant |
|----------|-------|-------|-------------------|
| input.cpp:123,125 | 700, 1300 | 3-pos switch thresholds | CRSF_SWITCH_LOW_MAX, CRSF_SWITCH_MID_MAX |
| input.cpp:292 | 2.0f | Weapon scaling | WEAPON_SCALE_FACTOR |
| input.cpp:365 | 74 | Nominal voltage | NOMINAL_VOLTAGE_DV |
| input.cpp:351 | 11 | Frame length | BATTERY_FRAME_LENGTH |

## SOLID Analysis

### Single Responsibility
**Status**: WARN

| Component | Responsibility | Assessment |
|-----------|---------------|------------|
| input.cpp | CRSF parsing | OK |
| input.cpp | Telemetry TX | OK |
| input.cpp | Input normalization | OK |
| crsf_process_byte() | Too many: sync, parse, validate, decode | **TOO BROAD** |

**Issue**: The `crsf_process_byte()` function handles frame synchronization, parsing, CRC validation, AND input decoding. Should be split.

### Open/Closed
**Status**: PASS

- Adding new channel mappings requires modification (acceptable for embedded)
- Frame types are extensible
- State machine is closed to modification

### Dependency Inversion
**Status**: PASS

- Depends on RuntimeState abstraction
- Uses Serial interface appropriately

## Embedded Memory Analysis

### Stack Usage

| Function | Local Stack | Worst Case | Status |
|----------|-------------|------------|--------|
| crsf_process_byte() | 4 bytes | 4 bytes | OK |
| crsf_send_battery_telemetry() | 18 bytes (packet[16] + 2 vars) | 18 bytes | OK |
| crsf_unpack_channels() | 0 bytes (inline optimized) | 0 bytes | EXCELLENT |
| **Total Max** | **18 bytes** | **22 bytes** | **OK** |

### Heap Allocations

| Location | Size | Status |
|----------|------|--------|
| None | - | EXCELLENT |

### Static Memory

| Type | Size | Description |
|------|------|-------------|
| .rodata (PROGMEM) | 256 bytes | CRC lookup table (in flash!) |
| .bss | 168 bytes | crsf_parser state + channels_raw |
| .bss | 4 bytes | last_telemetry_ms |
| **Total RAM** | **172 bytes** | **~8.4% of 2KB** |

**Assessment**: Excellent use of PROGMEM for CRC table (saves 256 bytes RAM). Static parser state is acceptable.

## Memory Safety

**Status**: GOOD

| Category | Status | Notes |
|----------|--------|-------|
| Buffer Safety | PASS | Frame buffer bounds checked |
| Array Access | PASS | Channel indices are constants |
| Null Checks | N/A | No pointers |
| Bounds Checks | PASS | Length validation before parsing |

### Specific Issues

**Strengths**:
- ✓ Frame length validation (lines 197-205)
- ✓ Bounds check on bytes_received (line 229)
- ✓ CRC validation before processing
- ✓ Fixed-size buffers (no dynamic allocation)

**Concerns**:
- ⚠️ No validation that payload is exactly 22 bytes for RC_CHANNELS
- ⚠️ Assumes frame_buffer is large enough (it is, but not explicitly checked)

## Embedded Best Practices

### Real-Time Constraints
**Status**: GOOD

- ✓ Non-blocking UART reads (Serial.available())
- ✓ Processes one byte at a time (bounded execution)
- ✓ Telemetry rate-limited to 1 Hz
- ⚠️ No worst-case execution time analysis for crsf_process_byte()

### Concurrency Safety
**Status**: PASS

- ✓ No ISR interactions
- ✓ All state access is atomic (8-bit AVR)
- ✓ Parser state is static (single thread)

### Power Efficiency
**Status**: PASS

- ✓ No busy waiting
- ✓ Serial.available() is efficient
- ✓ Telemetry rate-limited (not continuous)

## Issues Found

### CRITICAL (Must Fix)
None

### HIGH (Should Fix)

| ID | Category | Location | Issue | Fix |
|----|----------|----------|-------|-----|
| H1 | Complexity | input.cpp:180-328 | crsf_process_byte() CC=15, 147 lines, nesting=5 | Split into sub-functions |

### MEDIUM (Consider Fixing)

| ID | Category | Location | Issue | Fix |
|----|----------|----------|-------|-----|
| M1 | DRY | input.cpp:419-427 | Duplicated input reset pattern | Extract to reset_input_state() |
| M2 | Magic Numbers | input.cpp:123,125 | Hardcoded switch thresholds | Define as constants |
| M3 | Validation | input.cpp:264 | No check that RC_CHANNELS payload is 22 bytes | Add payload length validation |
| M4 | Error Recovery | input.cpp:320 | CRC error doesn't reset parser state | Consider resync on CRC failure |

### LOW (Optional)

| ID | Category | Location | Issue | Fix |
|----|----------|----------|-------|-----|
| L1 | Documentation | input.cpp:52-74 | Channel unpacking logic is dense | Add bit-position comments |
| L2 | Code Style | input.cpp:281-297 | Long expression chain | Break into intermediate variables |
| L3 | Naming | crsf_parser | Global-like name | Prefix with g_ or module_ |

## Detailed Issue Analysis

### H1: crsf_process_byte() Complexity
**Location**: input.cpp:180-328
**Severity**: HIGH

This function has:
- Cyclomatic Complexity: 15 (limit: 10)
- Cognitive Complexity: 18 (limit: 15)
- Function Length: 147 lines (limit: 50)
- Nesting Depth: 5 (limit: 4)

**Impact**: Difficult to test, maintain, and verify correctness.

**Recommendation**: Split into sub-functions:
```cpp
// State machine handlers
static void handle_address_byte(uint8_t byte);
static void handle_length_byte(uint8_t byte);
static void handle_type_byte(uint8_t byte);
static void handle_payload_byte(uint8_t byte);
static void handle_crc_byte(uint8_t byte);
static void process_rc_channels_frame(const uint8_t* payload);
```

**Effort**: Medium (2-3 hours)
**Priority**: HIGH (impacts maintainability)

### M3: Payload Length Validation
**Location**: input.cpp:264
**Severity**: MEDIUM

The code assumes RC_CHANNELS payload is 22 bytes but doesn't validate this.

**Current**:
```cpp
if (frame_type == CRSF_FRAMETYPE_RC_CHANNELS) {
    const uint8_t *payload = &crsf_parser.frame_buffer[3];
    crsf_unpack_channels(payload, crsf_parser.channels_raw);
}
```

**Recommendation**:
```cpp
if (frame_type == CRSF_FRAMETYPE_RC_CHANNELS) {
    if (crsf_parser.frame_length == CRSF_RC_CHANNELS_PAYLOAD_SIZE + 2) {
        const uint8_t *payload = &crsf_parser.frame_buffer[3];
        crsf_unpack_channels(payload, crsf_parser.channels_raw);
    } else {
        // Malformed packet - ignore
    }
}
```

## Recommendations

### Immediate Actions Required

1. **Refactor crsf_process_byte()** (H1)
   - Split into smaller state-machine handlers
   - Target: CC < 10 per function
   - Effort: 2-3 hours

### Short-Term Improvements

1. **Extract input reset pattern** (M1) - 30 min
2. **Add payload length validation** (M3) - 15 min
3. **Define switch threshold constants** (M2) - 10 min

### Technical Debt Paydown

1. Add unit tests for bit unpacking logic
2. Document bit positions in channel unpacking
3. Consider state machine table-driven approach

## Embedded-Specific Considerations

### Protocol Implementation Quality
**Status**: EXCELLENT

- ✓ Correct CRC-8-DVB-S2 implementation
- ✓ Proper 11-bit unpacking
- ✓ Frame synchronization with resync
- ✓ PROGMEM usage for lookup table

### Real-Time Performance
**Status**: GOOD

- ✓ Processes one byte per call (bounded time)
- ✓ No blocking waits
- ⚠️ Complex state machine could benefit from profiling

### Memory Efficiency
**Status**: EXCELLENT

- ✓ CRC table in PROGMEM (saves 256 bytes)
- ✓ Static parser state (no heap)
- ✓ Efficient bit packing/unpacking

### Production Readiness
**Status**: READY (with recommendation)

Code is functional and tested, but refactoring H1 would significantly improve long-term maintainability.

---
**Evaluated**: 2025-12-26
**Agent**: embedded-quality-evaluator
**Files Reviewed**: 2
**Lines Analyzed**: 489
