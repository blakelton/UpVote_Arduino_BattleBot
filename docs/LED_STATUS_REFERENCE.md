# UpVote Battlebot - LED Status Patterns Reference

**Version**: 1.0
**Firmware**: Phases 1-6 Complete
**Last Updated**: 2025-12-26

---

## Quick Reference

| Pattern | Status | Meaning | Action |
|---------|--------|---------|--------|
| **Slow Blink (1 Hz)** | SAFE | Disarmed, link healthy | Normal operation ✓ |
| **Fast Blink (5 Hz)** | ARMED | Weapon is live | Exercise caution ⚠️ |
| **Solid ON** | FAILSAFE | Kill switch or link loss | Check transmitter 🔴 |
| **Blink Pattern** | ERROR | System error detected | See error codes below 🔴 |

---

## Detailed LED Patterns

### 1. Slow Blink (1 Hz) - SAFE Mode

**Pattern**: ON for 500ms, OFF for 500ms (repeating)

```
ON  ████████          ████████          ████████
OFF         ████████          ████████
    |----1s----|----1s----|----1s----|
```

**Meaning**:
- System operational
- Weapon DISARMED (safe)
- Link to transmitter healthy
- No system errors

**When You See This**:
- ✓ Normal power-on state
- ✓ After disarming weapon (SA switch DOWN)
- ✓ During pre-match setup
- ✓ Safe to handle bot

**What To Do**:
- This is the expected state before arming
- Verify transmitter connection
- Perform control checks if needed

---

### 2. Fast Blink (5 Hz) - ARMED Mode

**Pattern**: ON for 100ms, OFF for 100ms (repeating)

```
ON  ██  ██  ██  ██  ██  ██  ██  ██
OFF   ██  ██  ██  ██  ██  ██  ██  ██
    |----1s----|
```

**Meaning**:
- System operational
- Weapon ARMED (live)
- Link to transmitter healthy
- Weapon slider controls spin speed

**When You See This**:
- After arming sequence (SA switch UP, slider at 0%)
- During match operation
- ⚠️ DANGER: Weapon may spin at any moment

**What To Do**:
- Bot is in combat mode
- Keep clear of weapon
- Use kill switch (SD UP) to disarm immediately if needed
- Monitor battery voltage on transmitter

---

### 3. Solid ON - FAILSAFE Mode

**Pattern**: Continuously ON (no blinking)

```
ON  ████████████████████████████████
OFF
```

**Meaning**:
- Emergency failsafe active
- All outputs disabled (motors, weapon, servo stopped)
- Triggered by:
  - Kill switch activated (SD switch UP)
  - Link loss to transmitter (>200ms timeout)

**When You See This**:

**A. If You Activated Kill Switch (SD UP)**:
- ✓ Expected behavior
- System is in safe state
- To recover: Move SD switch DOWN

**B. If Kill Switch is DOWN (Unexpected)**:
- 🔴 Link loss detected
- Check transmitter is ON
- Check transmitter battery
- Verify correct model selected on TX16S
- Check for interference (Wi-Fi, Bluetooth, metal barriers)
- Move closer to bot

**What To Do**:
1. Move SD switch DOWN (if it's UP)
2. If link lost: Restore transmitter connection
3. Once link recovered: LED returns to slow/fast blink
4. Re-arm if needed (SA switch)

---

### 4. Error Blink Patterns - ERROR Mode

**Pattern**: N blinks, 1-second pause, repeat

```
Example: ERR_LOOP_OVERRUN (1 blink)
ON  ██          ██          ██
OFF   ████████    ████████    ████████
    |----1s----|----1s----|

Example: ERR_WATCHDOG (2 blinks)
ON  ██  ██          ██  ██
OFF   ██  ████████    ██  ████████
    |----1s----|----1s----|

Example: ERR_CRSF_TIMEOUT (3 blinks)
ON  ██  ██  ██          ██  ██  ██
OFF   ██  ██  ████████    ██  ██  ████████
    |----1s----|----1s----|
```

**Meaning**:
- System detected an error
- Number of blinks indicates error code
- System may still be partially operational
- See error code table below

---

## Error Code Reference

| Blinks | Error Code | Name | Cause | Severity | Action |
|--------|------------|------|-------|----------|--------|
| **1** | `ERR_LOOP_OVERRUN` | Loop Overrun | Control loop exceeded 10ms | **MEDIUM** | Continue match, report after |
| **2** | `ERR_WATCHDOG` | Watchdog Reset | System recovered from freeze | **HIGH** | Finish engagement, inspect after |
| **3** | `ERR_CRSF_TIMEOUT` | Link Timeout | No packet received >200ms | **HIGH** | Restore transmitter link |
| **4** | `ERR_CRSF_CRC` | CRC Error | Corrupted CRSF packet | **LOW** | Check for interference |

---

## Error Code Details

### ERR_LOOP_OVERRUN (1 Blink)

**Cause**: Control loop execution time exceeded 10ms (100 Hz target)

**Impact**:
- Control timing degraded
- May affect responsiveness
- Bot continues operating

**Action**:
1. Continue current engagement safely
2. Disarm weapon after engagement
3. After match: Reflash firmware
4. Report issue to developers

**Prevention**:
- Should not occur in normal operation
- May indicate memory corruption or hardware fault

---

### ERR_WATCHDOG (2 Blinks)

**Cause**: Watchdog timer reset the system (500ms timeout triggered)

**Impact**:
- System was frozen/stuck
- Automatic recovery performed
- Brief loss of control occurred
- State reset to safe defaults

**Action**:
1. Finish current engagement cautiously
2. Disarm weapon immediately after
3. Disconnect battery
4. Inspect for:
   - Loose connections
   - Damaged components
   - Memory corruption
5. Reflash firmware before next match

**Prevention**:
- Hardware issue (vibration, impact damage)
- Firmware bug (should not happen)

---

### ERR_CRSF_TIMEOUT (3 Blinks)

**Cause**: No valid CRSF packet received from receiver for >200ms

**Impact**:
- Link lost to transmitter
- Failsafe activated (motors stopped, weapon disarmed)
- Bot will not respond to controls

**Action**:
1. **Check transmitter**:
   - Is TX16S powered on?
   - Is battery charged?
   - Is correct model selected?
2. **Check receiver**:
   - Is CR8 Nano powered?
   - Are antennas connected?
   - Is receiver bound to TX16S?
3. **Check environment**:
   - Move closer to bot
   - Remove metal barriers
   - Check for interference (Wi-Fi, Bluetooth)
4. **Recovery**:
   - Once link restored, LED returns to normal
   - Re-arm if needed

**Prevention**:
- Maintain line-of-sight to bot
- Keep transmitter close (< 50 ft in arena)
- Ensure antennas not damaged
- Avoid operating near strong RF sources

---

### ERR_CRSF_CRC (4 Blinks)

**Cause**: Received CRSF packet with invalid CRC checksum

**Impact**:
- Single corrupted packet rejected
- Link still operational
- Bot continues normal operation
- May degrade to TIMEOUT if multiple CRC errors

**Action**:
1. **Occasional CRC errors** (< 1 per second):
   - Normal in noisy RF environment
   - Continue operation
   - Monitor for increased frequency
2. **Frequent CRC errors** (> 5 per second):
   - Check for interference
   - Check receiver antenna orientation
   - Check wiring for loose connections
   - May need to move away from interference source

**Prevention**:
- Keep receiver antennas perpendicular
- Route receiver wiring away from motors/ESCs
- Ensure good ground connections
- Avoid operating near Wi-Fi routers

---

## LED State Transitions

### Normal Power-On Sequence

```
1. Power connected → Slow Blink (SAFE)
   - Waiting for transmitter link
   - Weapon disarmed

2. Transmitter link established → Slow Blink continues
   - Link healthy
   - Ready to arm

3. Arming sequence (SA UP, slider 0%) → Fast Blink (ARMED)
   - Weapon is live
   - Slider controls weapon speed

4. Disarm (SA DOWN) → Slow Blink (SAFE)
   - Weapon disarmed
   - Safe state restored
```

### Failsafe Activation

```
SAFE/ARMED → Link loss detected → Solid ON (FAILSAFE)
                                      ↓
                              Link restored
                                      ↓
                              SAFE (Slow Blink)
                                      ↓
                              Re-arm if needed → ARMED (Fast Blink)
```

### Kill Switch Activation

```
SAFE/ARMED → SD switch UP → Solid ON (FAILSAFE)
                                ↓
                          SD switch DOWN
                                ↓
                          SAFE (Slow Blink)
                                ↓
                          Re-arm if needed → ARMED (Fast Blink)
```

### Error Detection

```
SAFE/ARMED → Error detected → Error Blink Pattern (N blinks)
                                      ↓
                              Error persists until:
                              - Power cycle
                              - Condition cleared
                                      ↓
                              Return to SAFE (Slow Blink)
```

---

## Troubleshooting LED Issues

### LED Not Blinking at All

**Possible Causes**:
- No power to Arduino
- Battery dead/disconnected
- Arduino damaged
- LED burned out

**Action**:
1. Check battery voltage (should be > 6.5V)
2. Check battery connector
3. Check Arduino power LED (separate from pin 13 LED)
4. Try different battery
5. If Arduino power LED is ON but status LED is OFF → reflash firmware

---

### LED Blinking Rapidly Then Stopping

**Possible Causes**:
- System crash
- Power brownout
- Watchdog reset loop

**Action**:
1. Disconnect battery immediately
2. Check for:
   - Short circuits
   - Damaged components
   - Loose wiring
3. Reconnect battery and observe pattern
4. If issue persists → reflash firmware
5. If still fails → hardware fault (replace Arduino)

---

### LED Pattern Changing Randomly

**Possible Causes**:
- Intermittent link loss
- Interference
- Loose receiver connection
- Memory corruption

**Action**:
1. Check all wiring connections
2. Check transmitter battery
3. Move away from interference sources
4. Rebind receiver if needed
5. Reflash firmware if issue persists

---

## LED Hardware Details

**LED Location**: Arduino UNO built-in LED (pin 13)
**Type**: Yellow SMD LED on Arduino board
**Brightness**: Not adjustable (hardware fixed)
**Visibility**: Visible through enclosure vents

**Note**: The LED is controlled by firmware and cannot be manually overridden without reflashing.

---

## Related Documentation

- **Operator Guide**: [OPERATOR_GUIDE.md](OPERATOR_GUIDE.md)
- **Troubleshooting**: [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
- **Competition Checklist**: [COMPETITION_CHECKLIST.md](COMPETITION_CHECKLIST.md)

---

**Document Version**: 1.0
**Firmware Version**: Phases 1-6 Complete
**Last Reviewed**: 2025-12-26
