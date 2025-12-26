# 🎮 EdgeTX Model Setup Checklist (TX16S – Holonomic Bot)

Follow this checklist exactly to match firmware expectations.

---

## 1️⃣ Create New Model
- Type: **Plane** (simplest mixing)
- Name: `BattleBot-Holonomic`

---

## 2️⃣ Stick Inputs

| Channel | Source |
|-------|-------|
| CH1 | Right Stick LR |
| CH2 | Right Stick UD |
| CH3 | Left Stick LR |

Add **20–35% Expo** to CH1–CH3.

---

## 3️⃣ Sliders & Switches

| Channel | Control |
|-------|--------|
| CH4 | LS (Weapon throttle) |
| CH5 | SA (ARM, 3-pos) |
| CH6 | SE (KILL, 3-pos) |
| CH7 | SH (Self-right, momentary) |
| CH8 | SB (Drive mode, 3-pos) |

---

## 4️⃣ Switch Output Mapping

### SA (ARM)
- SA↓ → -100%
- SA– → 0%
- SA↑ → +100% (unused / reserved)

### SE (KILL)
- SE↓ → -100%
- SE– → 0%
- SE↑ → +100%

### SB (Drive Mode)
- SB↓ → -100% (Beginner)
- SB– → 0%   (Normal)
- SB↑ → +100% (Aggressive)

---

## 5️⃣ Weapon Throttle (CH4)

- Source: LS
- Curve: gentle near bottom, steeper near top
- Ensure bottom range reaches exact minimum

---

## 6️⃣ Failsafe Settings (ELRS)

Set failsafe outputs to:

| Channel | Value |
|-------|------|
| CH1–CH3 | 0 |
| CH4 | 0 |
| CH5 | -100 |
| CH6 | +100 |
| CH7 | 0 |
| CH8 | 0 |

If “No pulses” is used instead, Arduino watchdog must be enabled.

---

## 7️⃣ Pre-Flight Check

- ARM switch OFF
- Weapon slider at bottom
- Kill switch accessible
- Power on → no movement

---

## ✅ Done When

- All channels move as expected in receiver monitor
- No channel jitters at center
- Kill switch stops everything instantly