# UpVote Battlebot - Operator Quick Start Guide

**Version**: 1.1 (3S Upgrade)
**Firmware**: Phases 1-6 Complete
**Last Updated**: 2025-12-29

---

## Table of Contents

1. [Pre-Match Setup](#pre-match-setup)
2. [Power-On Sequence](#power-on-sequence)
3. [Arming the Weapon](#arming-the-weapon)
4. [Operating the Bot](#operating-the-bot)
5. [Emergency Procedures](#emergency-procedures)
6. [Post-Match Shutdown](#post-match-shutdown)
7. [LED Status Indicators](#led-status-indicators)
8. [TX16S Control Mapping](#tx16s-control-mapping)

---

## Pre-Match Setup

### Hardware Checklist

- [ ] **Battery**: 3S LiPo fully charged (12.6V nominal)
- [ ] **Receiver**: CR8 Nano ExpressLRS bound to TX16S
- [ ] **Transmitter**: TX16S powered on, model selected
- [ ] **Mechanical**: All screws tight, weapon balanced
- [ ] **Wiring**: All connections secure, no exposed wires
- [ ] **Visual**: No visible damage from previous matches

### Transmitter Setup (TX16S)

**CRITICAL**: Verify these switch positions BEFORE powering on:

| Control | Position | Function |
|---------|----------|----------|
| **SA** (Arm Switch) | ↓ DOWN | DISARMED (safe) |
| **SD** (Kill Switch) | ↓ DOWN | INACTIVE (allow operation) |
| **SH** (Self-Right) | Released | Neutral position |
| **Left Stick** (Throttle/Yaw) | CENTERED | No rotation |
| **Right Stick** (Roll/Pitch) | CENTERED | No translation |
| **Slider** (Weapon) | BOTTOM | 0% throttle |

**Drive Mode** (SB Switch):
- ↓ **DOWN**: Beginner (50% speed, gentle)
- ↔ **MIDDLE**: Normal (80% speed, balanced) ← RECOMMENDED
- ↑ **UP**: Aggressive (100% speed, responsive)

---

## Power-On Sequence

### Step 1: Verify Safe State

1. **Transmitter ON**: TX16S powered, correct model selected
2. **Switch Check**: All switches in safe positions (see table above)
3. **Arena Clear**: Ensure no one is near the bot

### Step 2: Connect Battery

1. **Plug in 3S LiPo** to main power connector
2. **Observe LED** on Arduino UNO (pin 13):
   - Should **blink slowly** (1 Hz) = SAFE mode ✓
   - If **solid ON** or **rapid blink**, see [Troubleshooting](TROUBLESHOOTING.md)

### Step 3: Verify Link

1. **Check TX16S telemetry screen**:
   - Should show "Batt: 11.1V" (or actual voltage if monitored)
   - "Fuel/Remaining" should show ~80-85% (free RAM percentage)
   - If no telemetry, see [Troubleshooting](TROUBLESHOOTING.md)

2. **LED should remain slow blink** (1 Hz) = Link OK ✓

### Step 4: Control Check

**IMPORTANT**: Perform this with bot on blocks (wheels off ground)

1. **Test Translation**:
   - Move right stick → wheels should respond
   - Verify correct direction (forward/back/strafe)

2. **Test Rotation**:
   - Move left stick left/right → all wheels should rotate bot
   - Verify smooth rotation in both directions

3. **Test Self-Right** (if installed):
   - Press and hold **SH** button → servo should extend slowly (~1.6s)
   - Release **SH** → servo should return to neutral slowly
   - **DO NOT** cycle rapidly (can cause brownout)

4. **Weapon Check**:
   - **SA switch** should still be DOWN (disarmed)
   - **Slider** at bottom (0%)
   - Bot is now ready for placement in arena

---

## Arming the Weapon

**⚠️ DANGER**: Only perform arming sequence when:
- Bot is in the arena
- All personnel are clear
- Match is about to start

### Arming Preconditions

The weapon will **ONLY arm** when **ALL** conditions are met:

1. ✓ **ARM switch (SA)** in UP position
2. ✓ **Kill switch (SD)** in DOWN position (inactive)
3. ✓ **Weapon slider** at 0-3% (near bottom)
4. ✓ **Link healthy** (transmitter connected)
5. ✓ **No system errors** (LED slow blink)

### Arming Procedure

1. **Verify slider at bottom** (0% weapon throttle) ← CRITICAL
2. **Move SA switch UP** (to ARMED position)
3. **Observe LED**:
   - Should change to **fast blink** (5 Hz) = ARMED ✓
   - If still slow blink, check preconditions above

4. **Weapon is now live** - slider controls spin speed:
   - 0-10%: Idle (safe)
   - 10-50%: Moderate power
   - 50-100%: Full power (use carefully)

### Throttle Hysteresis (Safety Feature)

If you disarm with weapon slider > 3%:
- You **MUST** move slider below 10% before you can re-arm
- This prevents accidental re-arming at high throttle
- **LED will stay slow blink** until slider drops below 10%

---

## Operating the Bot

### Drive Controls (TX16S EdgeTX)

| Control | Function | Range | Notes |
|---------|----------|-------|-------|
| **Right Stick Y** | Forward/Backward | -100% to +100% | Main translation |
| **Right Stick X** | Strafe Left/Right | -100% to +100% | Holonomic drive |
| **Left Stick X** | Rotate Left/Right | -100% to +100% | Yaw control (70% scaled) |
| **Left Stick Y** | *(Unused)* | - | Holonomic doesn't use throttle |
| **Slider** | Weapon Speed | 0% to 100% | Only active when ARMED |

### Drive Modes (SB Switch)

**Beginner** (↓ DOWN):
- 50% max speed
- Gentle exponential curve (0.3)
- Easiest to control, best for learning

**Normal** (↔ MIDDLE): ← RECOMMENDED
- 80% max speed
- Moderate exponential curve (0.2)
- Good balance of control and power

**Aggressive** (↑ UP):
- 100% max speed
- Minimal exponential curve (0.1)
- Most responsive, requires skill

**Tip**: Start in **Normal** mode. Switch to **Aggressive** only when you need max speed.

### Combat Tips

1. **Weapon Management**:
   - Keep weapon at ~50% during positioning
   - Increase to 100% only for impacts
   - Weapon has soft-start (takes ~2 seconds 0→100%)

2. **Self-Righting**:
   - Press and hold **SH** to extend flipper
   - Takes ~1.6 seconds to fully extend
   - Release immediately after flip
   - **Avoid** holding extended (drains power)

3. **Defensive Driving**:
   - Use strafe to dodge without turning
   - Combine translation + rotation for advanced maneuvers
   - Keep weapon pointed at opponent

4. **Battery Management**:
   - Monitor telemetry "Batt" voltage
   - At 7.0V: Consider conservative driving
   - At 6.8V: Plan to finish match soon
   - At 6.5V: Critical - withdraw if possible

---

## Emergency Procedures

### Immediate Disarm (KILL SWITCH)

**SD switch UP** (KILL position):
- **Instant effect**:
  - Weapon disarms immediately
  - Drive motors **STOP**
  - Self-right servo returns to neutral
  - LED goes **SOLID ON** = FAILSAFE mode

**Use when**:
- Bot is out of control
- Emergency stop required
- End of match
- Any unsafe condition

**Recovery**:
1. Return **SD to DOWN** (inactive)
2. LED returns to slow blink
3. Re-arm if needed (SA switch)

### Link Loss (Automatic Failsafe)

If transmitter loses connection (>200ms):

**Automatic Actions**:
- Weapon disarms immediately
- Drive motors hold last command briefly, then stop
- Self-right servo returns to neutral
- LED goes **SOLID ON**
- Error code stored (ERR_CRSF_TIMEOUT)

**Recovery**:
1. Restore transmitter connection
2. LED returns to slow or fast blink (depending on arm state)
3. Bot is safe to continue

### Loop Overrun (Rare)

If control loop takes >10ms (shouldn't happen):

**Symptoms**:
- LED blinks error code pattern (1 blink, pause, repeat)
- Bot continues operating but with degraded timing

**Action**:
1. Finish current engagement safely
2. Disarm weapon
3. After match: Report issue, reflash firmware

---

## Post-Match Shutdown

### Proper Shutdown Sequence

1. **Disarm Weapon**:
   - Move **SA switch DOWN** (disarmed)
   - Move **weapon slider to 0%**
   - LED should return to slow blink

2. **Stop Movement**:
   - Center all sticks
   - Verify bot is stationary

3. **Kill Power** (optional):
   - Move **SD switch UP** (kill)
   - LED goes solid ON

4. **Disconnect Battery**:
   - Unplug 3S LiPo connector
   - LED turns off
   - **Wait 10 seconds** before handling weapon

5. **Post-Match Inspection**:
   - Check for damage
   - Verify all screws tight
   - Test weapon balance manually

### Battery Care

- **Never** leave LiPo connected when not in use
- **Charge** to 12.6V (full) before next match
- **Storage charge** (11.4V = 3.8V/cell) if not using within 3 days
- **Dispose** if puffed or damaged

---

## LED Status Indicators

**Pin 13 LED** (on Arduino UNO):

| Pattern | Meaning | Action |
|---------|---------|--------|
| **Slow Blink (1 Hz)** | SAFE - Disarmed, link OK | Normal operation ✓ |
| **Fast Blink (5 Hz)** | ARMED - Weapon is live | Exercise caution ⚠️ |
| **Solid ON** | FAILSAFE - Kill switch or link loss | Check transmitter 🔴 |
| **N blinks, pause, repeat** | ERROR - System error code N | See [Troubleshooting](TROUBLESHOOTING.md) 🔴 |

**Error Code Reference**:

| Blinks | Error | Meaning |
|--------|-------|---------|
| 1 | Loop Overrun | Control loop took >10ms |
| 2 | Watchdog Reset | System recovered from freeze |
| 3 | CRSF Timeout | Link loss >200ms |
| 4 | CRSF CRC Error | Corrupted data packet |

For detailed troubleshooting, see [TROUBLESHOOTING.md](TROUBLESHOOTING.md).

---

## TX16S Control Mapping

### Complete Channel Map

| Channel | Control | Source | Function | Range |
|---------|---------|--------|----------|-------|
| **CH1** | Roll | Right Stick X | Strafe Left/Right | -100% to +100% |
| **CH2** | Pitch | Right Stick Y | Forward/Backward | -100% to +100% |
| **CH3** | Throttle | Left Stick Y | *(Unused)* | - |
| **CH4** | Yaw | Left Stick X | Rotate Left/Right | -100% to +100% |
| **CH5** | Weapon | Slider (S1 or S2) | Weapon Speed | 0% to 100% |
| **CH6** | ARM | SA (3-pos switch) | Weapon Arming | OFF/ON |
| **CH7** | Self-Right | SH (momentary) | Flipper Extend | Momentary |
| **CH8** | Drive Mode | SB (3-pos switch) | Speed Profile | BEGINNER/NORMAL/AGGRESSIVE |
| **CH9** | Kill Switch | SD (2-pos switch) | Emergency Stop | ACTIVE/INACTIVE |

### Switch Details

**SA (Arm Switch)** - 3-position:
- ↓ **DOWN**: DISARMED (safe)
- ↔ **MIDDLE**: DISARMED (safe)
- ↑ **UP**: ARMED (weapon enabled)

**SD (Kill Switch)** - 2-position:
- ↓ **DOWN**: Inactive (normal operation)
- ↑ **UP**: ACTIVE (emergency stop - all outputs safe)

**SH (Self-Right)** - Momentary button:
- **Released**: Servo at neutral
- **Pressed**: Servo extends (hold during flip)

**SB (Drive Mode)** - 3-position:
- ↓ **DOWN**: Beginner (50% speed)
- ↔ **MIDDLE**: Normal (80% speed) ← RECOMMENDED
- ↑ **UP**: Aggressive (100% speed)

---

## Quick Reference Card

**EMERGENCY**:
- **STOP EVERYTHING** → SD switch UP (KILL)
- **DISARM WEAPON** → SA switch DOWN
- **FAILSAFE ACTIVE** → Check transmitter, LED solid ON

**PRE-MATCH**:
1. Transmitter ON, verify switches safe
2. Connect battery → LED slow blink
3. Control check (on blocks)
4. Place in arena

**ARMING**:
1. Slider at 0%
2. SA switch UP
3. LED fast blink = ARMED

**DRIVING**:
- Right stick = translate (forward/strafe)
- Left stick = rotate
- Slider = weapon speed (when armed)
- SH button = self-right (hold briefly)

**POST-MATCH**:
1. SA DOWN (disarm)
2. Center sticks
3. SD UP (kill) - optional
4. Disconnect battery

---

## Safety Reminders

⚠️ **NEVER**:
- Arm weapon outside the arena
- Handle bot with weapon armed
- Leave battery connected unattended
- Operate with damaged components
- Bypass safety interlocks

✓ **ALWAYS**:
- Verify transmitter connection before power-on
- Keep weapon slider at 0% until ready
- Use kill switch (SD) in emergencies
- Disconnect battery after match
- Inspect for damage between matches

---

## Support & References

- **Troubleshooting Guide**: [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
- **LED Patterns Reference**: [LED_STATUS_REFERENCE.md](LED_STATUS_REFERENCE.md)
- **Competition Checklist**: [COMPETITION_CHECKLIST.md](COMPETITION_CHECKLIST.md)
- **Technical Documentation**: See `docs/` directory
- **Firmware Source**: GitHub repository

**Hardware Support**:
- TX16S EdgeTX Manual: https://edgetx.org
- ExpressLRS Documentation: https://expresslrs.org
- Arduino UNO Reference: https://docs.arduino.cc

---

**Document Version**: 1.0
**Firmware Version**: Phases 1-6 Complete
**Last Reviewed**: 2025-12-26

*This guide assumes proper hardware assembly and firmware upload. For build instructions, see hardware documentation.*
