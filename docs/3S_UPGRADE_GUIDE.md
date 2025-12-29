# UpVote Battlebot - 3S LiPo Upgrade Guide

**Version**: 1.0
**Firmware**: Phases 1-6 Complete (3S Compatible)
**Last Updated**: 2025-12-29

---

## Overview

This guide documents the upgrade from 2S LiPo (7.4V nominal) to 3S LiPo (11.1V nominal) battery configuration for the UpVote battlebot.

### What Changed

**Power System**:
- **Battery**: 2S LiPo (7.4V) → 3S LiPo (11.1V)
- **Voltage Increase**: +50% (from 7.4V to 11.1V nominal)
- **Weapon Performance**: ~2.25× kinetic energy (velocity squared relationship)

**Documentation Updated**:
- [WIRING_DIAGRAM.md](WIRING_DIAGRAM.md) - All voltage references, L293D shield details
- [OPERATOR_GUIDE.md](OPERATOR_GUIDE.md) - Battery checklist and telemetry values
- [COMPETITION_CHECKLIST.md](COMPETITION_CHECKLIST.md) - Voltage checks
- [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Voltage thresholds
- [Hardware Spec.md](initial_plan/Hardware%20Spec.md) - Power architecture
- [README.md](../README.md) - System specifications

**Firmware Updated**:
- `src/input.cpp` line 408: Telemetry voltage changed from 74 (7.4V) to 111 (11.1V)

---

## Table of Contents

1. [Quick Reference](#quick-reference)
2. [Battery Specifications](#battery-specifications)
3. [Component Compatibility](#component-compatibility)
4. [Performance Impact](#performance-impact)
5. [Voltage Reference Table](#voltage-reference-table)
6. [Upgrade Checklist](#upgrade-checklist)
7. [Testing Procedures](#testing-procedures)
8. [Safety Considerations](#safety-considerations)
9. [Troubleshooting](#troubleshooting)

---

## Quick Reference

### At a Glance

| Parameter | 2S (Old) | 3S (New) | Change |
|-----------|----------|----------|--------|
| **Nominal Voltage** | 7.4V | 11.1V | +50% |
| **Fully Charged** | 8.4V | 12.6V | +50% |
| **Minimum Safe** | 6.0V | 9.0V | +50% |
| **Storage Voltage** | 7.6V | 11.4V | +50% |
| **Critical Low** | 6.0V | 9.0V | +50% |
| **Weapon Kinetic Energy** | 1.0× | ~2.25× | +125% |

### Key Voltages for Operators

**Pre-Match Battery Check**:
- ✅ **Full Charge**: 12.6V ± 0.1V
- ⚠️ **Acceptable**: 11.1V - 12.6V
- ❌ **Charge First**: < 10.5V

**During Match Monitoring**:
- ✅ **Normal**: > 10.5V
- ⚠️ **Conservative Driving**: 9.5V - 10.5V
- 🔴 **Plan to Withdraw**: < 9.5V
- 🔴 **Critical**: < 9.0V (stop using battery)

**Post-Match**:
- ✅ **Reusable**: > 10.5V (can charge and reuse)
- ⚠️ **Charge Immediately**: 9.0V - 10.5V
- ❌ **Retire Battery**: < 9.0V (permanent damage likely)

---

## Battery Specifications

### 3S LiPo Characteristics

**Cell Configuration**:
- **Cells**: 3 in series
- **Per-Cell Voltage**:
  - Fully charged: 4.2V
  - Nominal: 3.7V
  - Storage: 3.8V
  - Minimum safe: 3.0V

**Pack Voltage**:
- **Fully Charged**: 12.6V (3× 4.2V)
- **Nominal**: 11.1V (3× 3.7V)
- **Storage**: 11.4V (3× 3.8V)
- **Minimum Safe**: 9.0V (3× 3.0V)

**Recommended Specifications**:
- **Capacity**: 1000-2200 mAh (typical for battlebot)
- **Discharge Rate**: 25C minimum (30C-50C recommended)
- **Connector**: XT60 or XT30 (high-current rated)
- **Form Factor**: Fit within bot chassis (measure before buying)

### Storage and Charging

**Charging**:
- **Charger Type**: LiPo balance charger (3S mode)
- **Charge Rate**: 1C maximum (1000mAh battery = 1A charge current)
- **Charge Voltage**: 12.6V (4.2V per cell)
- **Always balance charge**: Prevents cell imbalance

**Storage**:
- **Short-term** (< 3 days): Keep at full charge (12.6V)
- **Long-term** (> 3 days): Storage charge to 11.4V (3.8V per cell)
- **Storage temperature**: 20-25°C (68-77°F)
- **Storage location**: LiPo-safe bag or metal container

**Safety**:
- ❌ **Never** leave LiPo connected to bot unattended
- ❌ **Never** charge unattended
- ❌ **Never** store in hot environments (car, direct sunlight)
- ❌ **Never** use puffed or damaged batteries
- ✅ **Always** inspect before charging
- ✅ **Always** use balance charging
- ✅ **Always** charge in fire-safe bag

---

## Component Compatibility

### All Components 3S Compatible ✅

**Power Components**:

| Component | Voltage Range | 3S (11.1V) Status |
|-----------|---------------|-------------------|
| **MEIVIFPV BEC** | 2S-6S (7.4V - 22.2V) | ✅ **Perfect** |
| **L293D Motor Shield** | 4.5V - 36V | ✅ **Perfect** (11.1V well within range) |
| **HW30A ESC** | 2S-4S (7.4V - 14.8V) | ✅ **Perfect** |
| **RS2205 Motor** | 2S-4S rated | ✅ **Perfect** (faster spin = more power) |
| **TT Motors (Drive)** | Up to 12V | ✅ **Safe** (firmware limits to 80% = 8.9V effective) |
| **Arduino UNO** | 7-12V Vin (5V regulated) | ✅ **Perfect** (BEC provides 5V) |
| **CR8 Nano Receiver** | 5V (from BEC) | ✅ **Perfect** (unchanged) |
| **Self-Right Servo** | 5V (from BEC) | ✅ **Perfect** (unchanged) |

### Detailed Analysis

**MEIVIFPV BEC (5V/3A)**:
- Input range: 2S-6S (7.4V - 22.2V)
- 3S @ 11.1V is well within spec
- Output: 5V @ 3A (unchanged from 2S)
- Powers: Arduino VIN, CR8 Nano RX, self-right servo
- **No changes needed** ✅

**L293D Motor Shield**:
- Motor voltage input: 4.5V - 36V
- 3S @ 11.1V is **ideal** (middle of range)
- Current per channel: 600mA continuous, 1.2A peak
- TT motors receive PWM-averaged voltage (not full 11.1V)
- Firmware limits drive to 80% duty cycle (8.9V effective max)
- **Thermal note**: Chip may run slightly warmer on 3S vs 2S
  - L293D efficiency decreases with voltage differential
  - Monitor chip temperature during extended operation
  - Heatsink recommended if excessive heat observed
- **No changes needed** ✅

**HW30A ESC (Weapon)**:
- Input: 2S-4S (7.4V - 14.8V)
- 3S @ 11.1V is well within spec
- BEC disabled (not used)
- **Weapon motor gets full 3S voltage** (11.1V vs 7.4V)
- **Performance**: ~50% faster spin, **2.25× kinetic energy**
- **No changes needed** ✅

**RS2205 Brushless Motor (Weapon)**:
- Rated: 2S-4S
- KV rating: 2300-2600 (typical)
- On 3S: ~25,650 - 28,860 RPM (vs ~17,000 - 19,000 RPM on 2S)
- **Kinetic Energy**: E = ½Iω² (where ω is angular velocity)
  - 50% faster spin → (1.5)² = 2.25× more energy
  - **This is a significant increase** ⚠️
- **No changes needed** ✅ (but be aware of power increase)

**TT Motors (Drive)**:
- Rated: 3V - 12V (typical)
- 3S @ 11.1V would be near max rating
- **Protection**: Firmware limits drive to 80% PWM duty cycle
  - Effective max voltage: 11.1V × 0.80 = 8.9V
  - Well within safe operating range
- **PWM Averaging**: Motors receive time-averaged voltage, not full rail voltage
- **No changes needed** ✅

**Arduino UNO (ATmega328P)**:
- Powered by BEC @ 5V (unchanged)
- VIN not used (BEC connects to 5V pin)
- **No changes needed** ✅

**CR8 Nano Receiver**:
- Powered by BEC @ 5V (unchanged)
- **No changes needed** ✅

**Self-Right Servo**:
- Powered by BEC @ 5V (unchanged)
- **No changes needed** ✅

---

## Performance Impact

### Weapon Performance

**Kinetic Energy Increase**:
- **Formula**: E = ½Iω² (where I = moment of inertia, ω = angular velocity)
- **2S**: ω₁ = 7.4V × KV = ~17,000 RPM
- **3S**: ω₂ = 11.1V × KV = ~25,650 RPM
- **Ratio**: (25,650 / 17,000)² = (1.51)² ≈ **2.28× more kinetic energy**

**Practical Impact**:
- **Spin-Up Time**: Slightly faster (higher voltage, same motor)
- **Impact Force**: Significantly higher (~2.25× more energy transfer)
- **Battery Drain**: Weapon will draw ~50% more current (P = VI)
- **Match Duration**: May see 10-20% shorter run time if weapon at 100% constantly

**Recommendations**:
- Start at 50-70% weapon throttle during positioning
- Use 100% throttle only for impacts
- Weapon soft-start still applies (~2 seconds 0→100%)
- Monitor battery voltage more closely during matches

### Drive Performance

**No Significant Change**:
- Firmware limits drive to 80% duty cycle (8.9V effective on 3S vs 6.7V on 2S)
- ~33% higher effective drive voltage
- **Practical impact**: Slightly peppier acceleration, same top speed (limited by motor KV)
- **Battery drain**: Minimal increase (TT motors are low power)

**Recommendations**:
- No changes to driving technique needed
- Drive modes (Beginner/Normal/Aggressive) unchanged
- May feel slightly more responsive in Beginner mode

---

## Voltage Reference Table

### Complete Voltage Chart

| Voltage | Status | Meaning | Action |
|---------|--------|---------|--------|
| **12.6V - 12.7V** | ✅ Full Charge | Battery fully charged (4.2V/cell) | Ready for match |
| **12.0V - 12.6V** | ✅ Good | Recently charged or lightly used | Ready for match |
| **11.1V - 12.0V** | ✅ Nominal | Normal operating voltage | Ready for match |
| **10.5V - 11.1V** | ⚠️ Moderate Use | Battery moderately discharged | Check before match |
| **10.0V - 10.5V** | ⚠️ Low | Battery significantly discharged | Charge before next match |
| **9.5V - 10.0V** | 🔴 Very Low | Nearing minimum safe voltage | Conservative driving, plan to withdraw |
| **9.0V - 9.5V** | 🔴 Critical | At minimum safe voltage (3.0V/cell) | Stop using, charge immediately |
| **< 9.0V** | ❌ Damaged | Below safe voltage (cell damage likely) | Retire battery, do not reuse |

### Telemetry Display (TX16S)

**What You'll See**:
- **Pre-Match** (battery connected, idle): "Batt: 11.1V - 12.6V"
- **During Match** (under load): "Batt: 10.5V - 12.0V" (voltage sags under load)
- **Post-Match** (resting): "Batt: 9.5V - 11.5V" (depends on match intensity)

**Telemetry Update** (Firmware Change):
- **File**: `src/input.cpp` line 408
- **Old**: `voltage_dv = 74;` (reported 7.4V to TX16S)
- **New**: `voltage_dv = 111;` (reports 11.1V to TX16S)
- **Impact**: TX16S now displays correct nominal voltage for 3S
- **Note**: If `BATTERY_MONITOR_PIN` is defined, actual ADC voltage is reported instead

---

## Upgrade Checklist

### Pre-Upgrade (Before First 3S Use)

- [ ] **Verify 3S Battery**
  - Correct cell count (3 cells)
  - Fully charged (12.6V ± 0.1V)
  - No puffing or damage
  - Appropriate capacity (1000-2200 mAh)
  - High discharge rate (25C minimum)

- [ ] **Update Firmware** (CRITICAL)
  - Flash updated firmware with 3S telemetry change
  - Verify upload successful (serial monitor shows no errors)
  - Power cycle Arduino to ensure new firmware loaded

- [ ] **Update Documentation** (For Your Reference)
  - Print updated [OPERATOR_GUIDE.md](OPERATOR_GUIDE.md)
  - Print updated [COMPETITION_CHECKLIST.md](COMPETITION_CHECKLIST.md)
  - Bring [TROUBLESHOOTING.md](TROUBLESHOOTING.md) to competition

- [ ] **Transmitter Setup** (TX16S)
  - No changes needed (channel mapping unchanged)
  - Telemetry will now show 11.1V instead of 7.4V
  - Optional: Set low-voltage alarm to 9.5V

- [ ] **Component Inspection**
  - All wiring secure (no loose connections)
  - Battery connector clean and tight
  - ESC connections secure
  - No visible damage to components

### First 3S Test (On Blocks)

**CRITICAL: Perform this test with wheels off ground**

- [ ] **Safety Precautions**
  - Bot on blocks (wheels off ground)
  - Weapon disarmed (SA switch DOWN)
  - Weapon slider at 0%
  - TX16S powered on, correct model selected

- [ ] **Power-On Test**
  - Connect 3S battery (12.6V)
  - Observe LED → Should be **slow blink (1 Hz)** ✓
  - Check TX16S telemetry → Should show "Batt: 11.1V" ✓
  - If LED solid ON → Check transmitter connection

- [ ] **Drive Test**
  - Move right stick → Wheels should respond smoothly
  - Test all directions (forward, back, left, right)
  - Test rotation (left stick left/right)
  - No unusual sounds or binding

- [ ] **Weapon Test** (CAUTION)
  - Verify slider at 0%
  - Move SA switch UP → LED should change to **fast blink (5 Hz)** ✓
  - Slowly increase slider to 10% → Weapon should spin smoothly
  - Increase to 50% → Verify smooth acceleration
  - **DO NOT** test at 100% on blocks (too dangerous)
  - Return slider to 0%, SA switch DOWN

- [ ] **Thermal Check** (After 5 min operation)
  - Touch L293D chip on motor shield
  - Should be **warm** (body temperature to slightly hot)
  - If **very hot** (painful to touch) → Add heatsink or reduce drive time
  - Touch BEC → Should be warm, not hot
  - Touch HW30A ESC → Should be cool to warm

- [ ] **Telemetry Validation**
  - TX16S shows battery voltage (11.1V)
  - TX16S shows free RAM (~80-85%)
  - Telemetry updates every ~1 second

- [ ] **Failsafe Test**
  - Bot powered on, armed (fast blink LED)
  - Turn OFF transmitter
  - LED should go **solid ON** immediately ✓
  - Weapon should stop ✓
  - Turn ON transmitter → LED returns to fast blink ✓

### First Match Preparation

- [ ] **Pre-Match Checklist**
  - Follow [COMPETITION_CHECKLIST.md](COMPETITION_CHECKLIST.md) completely
  - Pay special attention to battery voltage checks (12.6V full charge)
  - Verify telemetry shows 11.1V (not 7.4V)

- [ ] **During Match Monitoring**
  - Watch telemetry voltage more closely (weapon uses more power)
  - At 10.0V → Consider conservative driving
  - At 9.5V → Plan to withdraw if possible
  - At 9.0V → Stop using battery immediately

- [ ] **Post-Match Inspection**
  - Check battery voltage (should be > 9.5V after rest)
  - Feel battery for heat (should be warm, not hot)
  - Check for puffing (discard if puffed)
  - Inspect L293D chip for overheating
  - Check weapon motor for unusual wear

---

## Testing Procedures

### Initial Bench Testing (2-3 hours)

**Goal**: Validate all systems work correctly on 3S before competition

**Test 1: Power-On and Link** (10 min)
1. TX16S on, correct model, switches safe
2. Connect 3S battery (12.6V)
3. LED slow blink ✓
4. TX16S telemetry shows 11.1V ✓
5. Pass criteria: LED blinks, telemetry correct

**Test 2: Drive Control** (15 min)
1. Bot on blocks (wheels off ground)
2. Test translation (right stick all directions)
3. Test rotation (left stick left/right)
4. Test drive modes (SB switch: Beginner/Normal/Aggressive)
5. Pass criteria: All wheels respond correctly, no binding

**Test 3: Weapon Control** (15 min)
1. Slider at 0%, SA switch UP (arm)
2. LED fast blink ✓
3. Slowly increase slider 0% → 50%
4. Weapon spins smoothly, no vibration
5. Test soft-start (slider 0% → 100% takes ~2 seconds)
6. Pass criteria: Weapon arms correctly, smooth spin

**Test 4: Self-Right Servo** (10 min)
1. Press and hold SH button
2. Servo extends slowly (~1.6 seconds)
3. Release SH → Servo returns to neutral
4. Pass criteria: Servo responds, no binding

**Test 5: Safety Interlocks** (15 min)
1. Failsafe test (turn off TX16S → LED solid ON)
2. Kill switch test (SD UP → LED solid ON, all outputs stop)
3. Arming preconditions (can't arm if slider > 3%)
4. Throttle hysteresis (can't re-arm if slider > 10%)
5. Pass criteria: All safety features work correctly

**Test 6: Thermal Testing** (30 min)
1. Run drive motors at 50% for 5 minutes
2. Run weapon at 50% for 2 minutes (on blocks, carefully)
3. Check component temperatures:
   - L293D chip: Warm to moderately hot (< 70°C)
   - BEC: Warm (< 50°C)
   - HW30A ESC: Cool to warm (< 60°C)
   - TT motors: Warm (< 60°C)
   - Weapon motor: Warm (< 80°C)
4. Let cool 5 minutes, repeat
5. Pass criteria: No excessive heat (nothing painfully hot)

**Test 7: Battery Life Test** (15 min)
1. Run full system (drive + weapon at 50%) for 10 minutes
2. Monitor telemetry voltage:
   - Start: 12.6V
   - 5 min: Should be > 11.5V
   - 10 min: Should be > 11.0V
3. Pass criteria: Battery voltage doesn't drop below 11.0V in 10 min

**Test 8: Stress Test** (20 min)
1. Simulate match conditions:
   - Drive aggressively (all directions, rotations)
   - Weapon at 70-100%
   - Self-right activations
   - Emergency stops (SD UP)
2. Run for 3 minutes, rest 2 minutes, repeat 3×
3. Monitor LED for error patterns
4. Check temperatures after each run
5. Pass criteria: No errors, no overheating

### Competition Day Testing

**Before Each Match** (10-15 min):
1. Follow [COMPETITION_CHECKLIST.md](COMPETITION_CHECKLIST.md) completely
2. Verify battery voltage: 12.6V ± 0.1V
3. Power-on test: LED slow blink, telemetry 11.1V
4. Control check (on blocks if possible)
5. Visual inspection: No damage, all screws tight

**After Each Match** (5-10 min):
1. Disconnect battery immediately
2. Check battery voltage (should be > 9.5V after rest)
3. Feel battery (warm OK, hot = problem)
4. Inspect for damage (armor, weapon, wiring)
5. Check L293D chip temperature
6. Recharge if voltage < 11.5V

---

## Safety Considerations

### Increased Weapon Energy ⚠️

**2.25× More Kinetic Energy**:
- Weapon hits will be **significantly more powerful**
- **Risk**: Opponent damage, self-damage from impacts
- **Mitigation**:
  - Start at 50% weapon throttle during testing
  - Use 100% only when confident in weapon mounting
  - Ensure weapon is **perfectly balanced** (spin test)
  - Check weapon fasteners with Loctite

**Recoil Forces**:
- Higher weapon speed = higher recoil on impact
- **Risk**: Bot flips itself on hard hits
- **Mitigation**:
  - Ensure low center of gravity
  - Test self-righting mechanism
  - Practice controlled impacts

### Thermal Considerations

**L293D Motor Shield**:
- May run warmer on 3S vs 2S
- **Risk**: Chip overheats, thermal shutdown, or damage
- **Mitigation**:
  - Monitor chip temperature during testing
  - Add heatsink if chip gets very hot (> 70°C)
  - Reduce continuous drive time if needed
  - Firmware already limits duty cycle to 80%

**Battery**:
- Weapon will draw more current on 3S
- **Risk**: Battery overheating, puffing, or fire
- **Mitigation**:
  - Use high-discharge batteries (25C minimum)
  - Never leave battery connected unattended
  - Feel battery after use (warm OK, hot = retire battery)
  - Charge in fire-safe bag

### Electrical Safety

**Higher Voltage**:
- 3S @ 11.1V is still safe for all components
- **Risk**: Slightly higher shock hazard (still low voltage)
- **Mitigation**:
  - Ensure all connections insulated
  - No exposed wires or connectors
  - Use proper gauge wire (14-16 AWG for main power)

**Short Circuit**:
- Higher voltage = higher short-circuit current
- **Risk**: Fire, component damage
- **Mitigation**:
  - Inspect wiring before every match
  - Use properly rated connectors (XT60/XT30)
  - Never work on bot with battery connected

### Competition-Specific Safety

**Arming Sequence**:
- **UNCHANGED** from 2S system
- Slider at 0%, SA switch UP, LED fast blink
- **Critical**: Never arm weapon outside arena

**Kill Switch**:
- **UNCHANGED** from 2S system
- SD switch UP immediately stops all outputs
- Practice using kill switch before first match

**Emergency Procedures**:
- See [OPERATOR_GUIDE.md](OPERATOR_GUIDE.md) Emergency Procedures section
- Link loss → LED solid ON, weapon stops automatically
- Out of control → SD switch UP immediately

---

## Troubleshooting

### Common Issues After 3S Upgrade

**Issue 1: LED Solid ON (Should Be Slow Blink)**

**Symptoms**:
- LED solid ON when battery connected
- No blinking pattern

**Diagnosis**:
1. TX16S powered on? → If no, turn on TX16S
2. Correct model selected? → If no, select correct model
3. CR8 Nano RX powered? → Check BEC output (5V between BEC red/black)
4. CR8 bound to TX16S? → Rebind if needed

**Fix**:
- See [TROUBLESHOOTING.md](TROUBLESHOOTING.md) "LED Solid ON" section

---

**Issue 2: Telemetry Shows 7.4V (Should Be 11.1V)**

**Symptoms**:
- TX16S telemetry shows "Batt: 7.4V" instead of 11.1V
- Firmware not updated

**Diagnosis**:
1. Firmware upload successful? → Check serial monitor for errors
2. Correct firmware file flashed? → Verify `src/input.cpp` line 408 = 111
3. Arduino power cycled after flash? → Disconnect battery, wait 10 seconds, reconnect

**Fix**:
1. Re-upload firmware: `platformio run --target upload`
2. Verify upload success in serial monitor
3. Disconnect battery, wait 10 seconds
4. Reconnect battery, check telemetry

---

**Issue 3: L293D Chip Very Hot**

**Symptoms**:
- L293D chip painfully hot to touch (> 70°C)
- After 5-10 minutes of driving

**Diagnosis**:
1. Continuous high-speed driving? → Thermal buildup normal under heavy use
2. Chip temperature > 70°C? → Measure with IR thermometer or finger (if < 2 seconds tolerable = OK)
3. Drive duty cycle > 80%? → Firmware should limit to 80%, check `config.h`

**Fix**:
1. **Short-term**: Reduce continuous drive time, use bursts instead
2. **Long-term**: Add heatsink to L293D chip (thermal adhesive or clip-on)
3. **Verify**: Firmware `NORMAL_SPEED_LIMIT = 0.80f` in `config.h`
4. **Alternative**: Switch to Aggressive mode (100%) only when needed, use Normal (80%) most of the time

---

**Issue 4: Battery Voltage Drops Quickly**

**Symptoms**:
- Voltage drops from 12.6V to < 11.0V in < 5 minutes
- Battery feels hot after use

**Diagnosis**:
1. Battery capacity too low? → Use 1500+ mAh for 3-5 min matches
2. Battery discharge rate too low? → Use 25C minimum (30C+ recommended)
3. Weapon at 100% constantly? → Weapon draws significant current on 3S
4. Battery puffed or damaged? → Retire battery if puffed

**Fix**:
1. **Immediate**: Use higher capacity battery (1500-2200 mAh)
2. **Immediate**: Use higher discharge rate battery (30C-50C)
3. **Operational**: Keep weapon at 50% during positioning, 100% only for impacts
4. **Safety**: Retire any puffed batteries immediately

---

**Issue 5: Weapon Won't Arm**

**Symptoms**:
- SA switch UP, but LED stays slow blink (doesn't go fast blink)
- Weapon doesn't spin

**Diagnosis**:
1. Weapon slider position? → Must be ≤ 3% to arm
2. Kill switch (SD) position? → Must be DOWN (inactive)
3. Link healthy? → LED slow blink = link OK, solid ON = link lost
4. System errors? → Check for error pattern (N blinks + pause)

**Fix**:
1. Move weapon slider to **bottom** (0%)
2. Verify SD switch **DOWN** (inactive)
3. Move SA switch UP → LED should change to fast blink
4. If still slow blink, see [TROUBLESHOOTING.md](TROUBLESHOOTING.md) "Weapon Won't Arm"

---

**Issue 6: Weapon Spins Unevenly or Vibrates**

**Symptoms**:
- Weapon wobbles when spinning
- Vibration felt in chassis
- Unusual noise

**Diagnosis**:
1. Weapon balanced? → Spin manually, should rotate smoothly without stopping at same position
2. Weapon fasteners tight? → Check all screws/bolts
3. Motor mount secure? → Check motor mounting screws
4. Weapon damaged? → Inspect for cracks, bends, or missing material

**Fix**:
1. **Balance weapon**: Add weight to lighter side or remove from heavier side
2. **Tighten fasteners**: Use Loctite on critical screws
3. **Inspect damage**: Replace weapon if cracked or bent
4. **Check motor**: Ensure motor shaft not bent, bearings not worn

---

**Issue 7: Drive Motors Not Responding**

**Symptoms**:
- Sticks moved on TX16S, but wheels don't spin
- LED blinks correctly (slow or fast)

**Diagnosis**:
1. L293D shield powered? → Check battery voltage at shield terminals (should be 11.1V)
2. L293D chip seated correctly? → Verify chip inserted fully in socket
3. Motor connections? → Check motor wires to shield terminals
4. Drive mode switch? → Try different drive modes (SB switch)

**Fix**:
1. Check battery connection to L293D shield motor power terminals
2. Reseat L293D chip (power off first!)
3. Verify motor wiring (see [WIRING_DIAGRAM.md](WIRING_DIAGRAM.md))
4. Test with different drive mode (SB switch)
5. See [TROUBLESHOOTING.md](TROUBLESHOOTING.md) "Motors Not Responding"

---

### When to Reflash Firmware

**Reflash Required**:
- ✅ After 3S upgrade (telemetry voltage change)
- ✅ If telemetry shows wrong voltage (7.4V instead of 11.1V)
- ✅ If unexplained errors occur after upgrade

**Reflash Procedure**:
```bash
# In project root directory
platformio run --target upload

# Verify upload success
platformio device monitor --baud 115200
```

**Verification**:
1. Connect battery → LED slow blink
2. Check TX16S telemetry → Should show 11.1V
3. Arm weapon → LED fast blink
4. Test all controls

---

## Additional Resources

### Documentation

- **[WIRING_DIAGRAM.md](WIRING_DIAGRAM.md)** - Complete electrical wiring (updated for 3S)
- **[OPERATOR_GUIDE.md](OPERATOR_GUIDE.md)** - Operating manual (updated for 3S)
- **[COMPETITION_CHECKLIST.md](COMPETITION_CHECKLIST.md)** - Pre/post-match procedures (updated for 3S)
- **[TROUBLESHOOTING.md](TROUBLESHOOTING.md)** - Diagnostic procedures (updated for 3S)
- **[LED_STATUS_REFERENCE.md](LED_STATUS_REFERENCE.md)** - LED pattern guide (unchanged)
- **[HARDWARE_TESTING_GUIDE.md](HARDWARE_TESTING_GUIDE.md)** - Systematic testing procedures

### External References

- **LiPo Battery Safety**: https://rogershobbycenter.com/lipoguide/
- **L293D Datasheet**: https://www.ti.com/lit/ds/symlink/l293d.pdf
- **Arduino UNO Reference**: https://docs.arduino.cc/hardware/uno-rev3
- **ExpressLRS Docs**: https://expresslrs.org
- **EdgeTX Manual**: https://edgetx.org

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2025-12-29 | Initial 3S upgrade guide creation |

---

## Summary

**3S Upgrade Complete** ✅

**What Changed**:
- Battery: 2S (7.4V) → 3S (11.1V)
- Weapon performance: ~2.25× more kinetic energy
- Firmware: Telemetry voltage updated (7.4V → 11.1V)
- Documentation: All voltage references updated

**What Stayed the Same**:
- Hardware: All components 3S compatible (no hardware changes needed)
- Wiring: No wiring changes needed
- Control mapping: TX16S channels unchanged
- Safety features: Arming, kill switch, failsafe all unchanged
- Operating procedures: Same arming sequence, same controls

**Next Steps**:
1. ✅ Flash updated firmware
2. ✅ Charge 3S battery to 12.6V
3. ✅ Run initial bench tests (2-3 hours)
4. ✅ Compete with confidence!

---

**Document Version**: 1.0
**Firmware Version**: Phases 1-6 Complete (3S Compatible)
**Last Reviewed**: 2025-12-29

**Ready to compete with 3S power! 🤖⚡**
