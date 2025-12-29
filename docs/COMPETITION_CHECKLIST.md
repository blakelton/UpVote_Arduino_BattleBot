# UpVote Battlebot - Competition Readiness Checklist

**Version**: 1.0
**Firmware**: Phases 1-6 Complete
**Last Updated**: 2025-12-26

---

## How to Use This Checklist

**Before Each Match**:
- ☐ Complete the **Pre-Event** section
- ☐ Complete the **Pre-Match** section
- ☐ Complete the **Post-Match** section after each fight

**Frequency**:
- **Pre-Event**: Once before competition day
- **Pre-Match**: Before EVERY match
- **Post-Match**: After EVERY match

**Time Estimates**:
- Pre-Event: 30-45 minutes
- Pre-Match: 10-15 minutes
- Post-Match: 5-10 minutes

---

## Pre-Event Checklist (Day Before / Morning Of)

### Electronics

- ☐ **Firmware Version Verified**
  - Check firmware version: Phases 1-6 complete
  - No experimental or debug code active
  - All safety features enabled

- ☐ **Arduino UNO Functional**
  - Power LED lights when connected
  - USB programming port works
  - No visible damage to board

- ☐ **CR8 Nano Receiver Bound**
  - Receiver bound to TX16S transmitter
  - Antennas attached and undamaged
  - Green LED on receiver blinks when TX16S is on

- ☐ **Transmitter (TX16S) Ready**
  - Battery fully charged (check voltage on screen)
  - Correct model selected ("UpVote" or similar)
  - Channel mapping verified (see OPERATOR_GUIDE.md)
  - All switches in safe positions (SA DOWN, SD DOWN)

- ☐ **Batteries Charged**
  - Main 3S LiPo fully charged (12.6V nominal)
  - At least 2 spare batteries charged
  - No puffing or damage
  - Storage temp: 20-25°C (68-77°F)

- ☐ **Battery Monitor (Optional)**
  - If installed: Voltage monitor calibrated
  - If not installed: Manual voltage checks planned

### Mechanical

- ☐ **Weapon Balanced**
  - Spin weapon manually - no wobble
  - Fasteners tight (Loctite if needed)
  - No cracks or damage

- ☐ **Wheels/Drive**
  - Wheels attached securely
  - No visible damage to tires
  - Drive motors spin freely when unpowered

- ☐ **Armor/Shell**
  - All screws tight
  - No cracks or loose panels
  - Weapon clearance adequate

- ☐ **Self-Right Mechanism (If Installed)**
  - Servo arm moves freely
  - Linkage secure
  - No binding

### Wiring

- ☐ **Main Power Connector**
  - XT60/XT30/Deans connector tight
  - No exposed wire
  - Polarity correct (test with multimeter)

- ☐ **Motor Connections**
  - ESC to motor wires secure
  - No frayed insulation
  - Bullet connectors tight

- ☐ **Receiver Wiring**
  - UART pins secure (Arduino RX to CR8 TX)
  - 5V power to receiver from BEC
  - Ground connected

- ☐ **Kill Switch (If External)**
  - Removable link installed
  - Link easily removable by officials
  - Switch functional

### Software

- ☐ **Control Test (On Blocks)**
  - Bot on blocks (wheels off ground)
  - Power on → Slow blink LED ✓
  - Right stick → Wheels respond correctly
  - Left stick → Bot rotates correctly
  - SA switch UP (with slider at 0%) → Fast blink LED ✓
  - Weapon slider → Weapon spins (if armed)
  - SD switch UP → Solid LED, all outputs stop ✓
  - SH button → Self-right servo extends (if installed) ✓

- ☐ **Failsafe Test**
  - Bot powered on and armed
  - Turn OFF transmitter
  - LED should go solid ON ✓
  - Weapon should stop ✓
  - Motors should stop ✓
  - Turn ON transmitter → LED returns to slow blink ✓

- ☐ **Telemetry Test (If Enabled)**
  - TX16S shows battery voltage on telemetry screen
  - TX16S shows free RAM percentage ("Fuel Remaining")
  - Values update every ~1 second

### Spares & Tools

- ☐ **Spare Parts**
  - At least 2 spare 3S LiPo batteries (charged)
  - Spare wheels (if applicable)
  - Spare fasteners (M3/M4 screws)
  - Spare weapon components (if available)

- ☐ **Tools On-Site**
  - Hex keys (2.5mm, 3mm, 4mm)
  - Screwdrivers (Phillips, flathead)
  - Multimeter
  - Battery charger + power adapter
  - Laptop with PlatformIO (for emergency reflash)
  - USB cable (Arduino programming)

- ☐ **Documentation Printed**
  - OPERATOR_GUIDE.md
  - TROUBLESHOOTING.md
  - LED_STATUS_REFERENCE.md
  - This checklist

---

## Pre-Match Checklist (Before EVERY Match)

**Time Limit**: Aim to complete in 10-15 minutes

### Battery & Power

- ☐ **Battery Fully Charged**
  - Check voltage: Should be 12.6V ± 0.1V (full charge)
  - If < 8.0V → Charge before match
  - No puffing or heat

- ☐ **Battery Connector Clean**
  - No debris in connector
  - Pins not bent
  - Connection firm (not loose)

### Mechanical Inspection

- ☐ **Visual Inspection**
  - No visible damage from previous match
  - All panels secure
  - Weapon fasteners tight

- ☐ **Weapon Check**
  - Spin manually → No binding
  - Balanced (no wobble)
  - Loctite still holding (if used)

- ☐ **Wheel Check**
  - All wheels attached
  - No tread damage
  - Rotate freely

### Electronics

- ☐ **Transmitter Ready**
  - TX16S powered on
  - Correct model selected
  - Battery > 30% (check screen)
  - All switches in safe positions:
    - SA (Arm): ↓ DOWN
    - SD (Kill): ↓ DOWN
    - Slider (Weapon): ↓ BOTTOM (0%)

- ☐ **Receiver Check**
  - Antennas intact (not bent/broken)
  - Green LED blinking (link active)

### Power-On Sequence

**CRITICAL**: Perform this EVERY time

1. ☐ **Verify Transmitter ON**
   - TX16S screen active
   - Correct model selected
   - All switches safe (SA DOWN, SD DOWN, slider BOTTOM)

2. ☐ **Connect Battery**
   - Plug in 3S LiPo
   - Observe Arduino LED → Should be **slow blink (1 Hz)**
   - If solid ON or fast blink → TROUBLESHOOT (see TROUBLESHOOTING.md)

3. ☐ **Verify Link**
   - TX16S telemetry screen shows battery voltage
   - LED still slow blinking
   - If no telemetry → Check receiver power

4. ☐ **Control Check (On Blocks If Possible)**
   - Right stick → Wheels respond
   - Left stick → Rotation works
   - **DO NOT arm weapon yet**

5. ☐ **Place in Arena**
   - Bot in safe state (slow blink LED)
   - SA switch DOWN (disarmed)
   - Weapon slider at 0%

### Pre-Match Staging

- ☐ **In Staging Area**
  - Bot powered on
  - Slow blink LED (disarmed)
  - Ready to carry into arena

- ☐ **Final Transmitter Check**
  - SA switch DOWN (disarmed)
  - SD switch DOWN (inactive)
  - Weapon slider at BOTTOM (0%)
  - Drive mode switch: MIDDLE (Normal) recommended

- ☐ **Mental Checklist**
  - Know the arming sequence (SA UP when slider at 0%)
  - Know kill switch location (SD UP)
  - Know opponent strategy (if applicable)

---

## In-Arena Checklist

**Time**: 1-2 minutes before match start

### Placement

- ☐ **Bot Placed in Starting Square**
  - LED slow blink (disarmed) ✓
  - Facing correct direction
  - All wheels on ground

### Match Ready

- ☐ **Transmitter in Hand**
  - Comfortable grip
  - All switches accessible
  - Eyes on bot

- ☐ **Verify Slow Blink**
  - LED slow blink (1 Hz) = SAFE ✓
  - If solid ON → Restore link (TX16S on?)
  - If fast blink → DISARM (SA DOWN)

### Arming Sequence

**⚠️ ONLY perform when officials give "Activate" command**

1. ☐ **Verify Weapon Slider at 0%**
   - Slider at bottom position
   - Critical safety check

2. ☐ **Move SA Switch UP**
   - Arm switch to UP position
   - Observe LED → Should change to **fast blink (5 Hz)**

3. ☐ **Confirm Armed**
   - LED fast blink = ARMED ✓
   - Weapon slider now controls speed
   - Ready for "3, 2, 1, Fight!"

---

## During Match

### Normal Operation

- ☐ **Monitor LED** (if visible)
  - Fast blink = Normal (armed)
  - Solid ON = Failsafe (restore link)
  - Error pattern = See LED_STATUS_REFERENCE.md

- ☐ **Monitor Transmitter Telemetry**
  - Battery voltage: Stay above 7.0V if possible
  - At 6.8V → Conservative driving
  - At 6.5V → Critical, plan to withdraw

- ☐ **Weapon Management**
  - Keep at ~50% during positioning
  - 100% only for impacts
  - Allows for battery headroom

### Emergency Situations

- ☐ **If Bot Out of Control**
  - SD switch UP (KILL) immediately
  - Verify LED solid ON
  - Notify officials

- ☐ **If Link Lost**
  - LED will go solid ON automatically
  - Check TX16S is on
  - Check transmitter battery
  - Restore link → LED returns to blink pattern

- ☐ **If Fire/Smoke**
  - SD switch UP (KILL)
  - Notify officials immediately
  - Do NOT approach bot
  - Wait for officials' instructions

---

## Post-Match Checklist

**Time**: 5-10 minutes after retrieval

### Immediate (In Arena)

- ☐ **Disarm Weapon**
  - SA switch DOWN (disarmed)
  - Weapon slider to 0%
  - LED slow blink (if link OK)

- ☐ **Kill Power (Optional)**
  - SD switch UP (kill)
  - LED solid ON
  - Safe to retrieve

- ☐ **Disconnect Battery**
  - Unplug 3S LiPo
  - **Wait 10 seconds before handling weapon**
  - ESC capacitors discharge

### Inspection (In Pit Area)

- ☐ **Damage Assessment**
  - Armor: Cracks, dents, missing screws?
  - Weapon: Bent, cracked, unbalanced?
  - Wheels: Tread damage, bent axles?
  - Wiring: Exposed wires, loose connections?

- ☐ **Mechanical Checks**
  - Spin weapon manually → Balanced?
  - Wheels rotate freely?
  - Self-right servo still works?

- ☐ **Electrical Checks**
  - All screws tight
  - No burnt smell
  - Connectors still secure

### Battery Management

- ☐ **Battery Condition**
  - Check voltage (should be > 7.0V after rest)
  - Feel for heat (should be warm, not hot)
  - Check for puffing (discard if puffed)

- ☐ **Recharge Decision**
  - If voltage < 7.5V → Charge for next match
  - If voltage > 7.5V → May use again (depends on schedule)
  - Always swap if time permits

### Repairs Needed?

- ☐ **Critical Repairs** (before next match)
  - Weapon damaged → Replace/repair
  - Drive not working → Diagnose/fix
  - Wiring exposed → Re-insulate
  - Armor missing → Reinstall

- ☐ **Non-Critical Repairs** (if time permits)
  - Cosmetic damage → Note for later
  - Wheel tread wear → Replace if severe
  - Fasteners loose → Retighten

### Data Collection (Optional)

- ☐ **Match Notes**
  - What worked well?
  - What needs improvement?
  - Opponent weaknesses observed?
  - Strategy for rematch?

- ☐ **Technical Notes**
  - Any errors observed? (LED patterns)
  - Battery voltage at end of match?
  - Drive power sufficient?
  - Weapon effectiveness?

---

## Between-Matches Checklist

**If you have multiple matches scheduled**

### Preparation

- ☐ **Battery Ready**
  - Fully charged 3S LiPo (12.6V)
  - Cool to touch (not hot from charger)

- ☐ **Repairs Complete**
  - All critical repairs done
  - Tested after repairs (power on, control check)

- ☐ **Transmitter Charged**
  - TX16S battery > 50%
  - Charge if needed

### Pre-Match Repeat

- ☐ **Run Pre-Match Checklist Again**
  - Don't skip steps (complacency kills)
  - Verify everything even if "it was fine last time"

---

## End-of-Day Checklist

### Shutdown

- ☐ **Battery Disconnected**
  - All batteries removed from bot
  - No LiPo left connected overnight

- ☐ **Battery Storage**
  - If competing next day → Keep at full charge
  - If done for weekend → Storage charge (7.6V) within 24 hours

### Maintenance

- ☐ **Clean Bot**
  - Remove debris from weapon
  - Check for hidden damage
  - Wipe down exterior

- ☐ **Inventory**
  - Count spare parts remaining
  - Note what needs replenishing

### Data Backup

- ☐ **Match Video** (if recorded)
  - Copy video files to laptop
  - Backup to cloud if possible

- ☐ **Notes Saved**
  - Transfer handwritten notes to digital
  - Update strategy document

---

## Emergency Contact Info

**Event Officials**:
- Safety Officer: _______________________
- Tech Inspection: _______________________

**Team Contacts**:
- Primary Operator: _______________________
- Pit Crew: _______________________

**Emergency Numbers**:
- Event First Aid: _______________________
- Local Emergency: 911 (or local equivalent)

---

## Quick Reference: LED Patterns

| Pattern | Meaning | Action |
|---------|---------|--------|
| Slow Blink (1 Hz) | SAFE (disarmed) | Normal ✓ |
| Fast Blink (5 Hz) | ARMED (weapon live) | Caution ⚠️ |
| Solid ON | FAILSAFE (kill/link loss) | Check TX 🔴 |
| N Blinks + Pause | ERROR (code N) | See LED_STATUS_REFERENCE.md 🔴 |

---

## Safety Reminders

**NEVER**:
- ❌ Arm weapon outside arena
- ❌ Handle bot with weapon armed (fast blink LED)
- ❌ Leave battery connected unattended
- ❌ Operate with visible damage
- ❌ Skip failsafe test

**ALWAYS**:
- ✅ Verify slow blink LED before handling
- ✅ Keep weapon slider at 0% until armed in arena
- ✅ Use kill switch (SD UP) in emergencies
- ✅ Disconnect battery after match
- ✅ Wait 10 seconds after power-off before touching weapon

---

## Troubleshooting During Competition

**For detailed troubleshooting, see [TROUBLESHOOTING.md](TROUBLESHOOTING.md)**

### Quick Fixes

**LED Not Blinking**:
1. Check battery voltage (> 7.0V?)
2. Check battery connector
3. Try different battery

**Link Not Working**:
1. TX16S powered on?
2. Correct model selected?
3. Receiver antennas intact?
4. Rebind receiver (if time permits)

**Weapon Won't Spin**:
1. Is bot armed? (fast blink LED?)
2. Is weapon slider > 10%?
3. Check weapon ESC connection
4. Check weapon motor connection

**Drive Not Working**:
1. Is link OK? (slow/fast blink LED?)
2. Are sticks centered when powered on?
3. Check drive mode (SB switch)
4. Check motor connections

---

## Related Documentation

- **Operator Guide**: [OPERATOR_GUIDE.md](OPERATOR_GUIDE.md)
- **Troubleshooting**: [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
- **LED Patterns**: [LED_STATUS_REFERENCE.md](LED_STATUS_REFERENCE.md)
- **Control Mapping**: TX16S channel map in OPERATOR_GUIDE.md

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2025-12-26 | Initial release for Phases 1-6 firmware |

---

**Document Version**: 1.0
**Firmware Version**: Phases 1-6 Complete
**Last Reviewed**: 2025-12-26

**Good luck in competition! 🤖⚔️**
