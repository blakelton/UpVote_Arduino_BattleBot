# 🤖 Battlebot Hardware Specification (2S System)


Ref: https://www.printables.com/model/897022-force-knight-1lb-plastic-antweight-combat-robot/files


## 🔋 Power Architecture
- **Battery:** 2S LiPo
- **Weapon Rail (Raw 2S):**
  - Battery ➜ HW30A ESC ➜ RS2205 Brushless Motor ⚔️
- **Drive Rail (Regulated):**
  - Battery ➜ Buck Converter ➜ **6.0V** ➜ L293D Motor Shield ➜ 4× TT Motors 🚗
- **Logic Rail (Clean Power):**
  - Battery ➜ 5V BEC ➜ Arduino UNO + ELRS Receiver + Servo 🧠

⚠️ Logic and motor power are intentionally separated to avoid resets and noise issues.

---

## 🎮 Control System
- **Transmitter:** RadioMaster TX16S
- **Receiver:** ELRS Nano Receiver
- **Controller:** Arduino UNO

### Signal Flow
- TX16S ➜ ELRS Receiver ➜ Arduino UNO
- Arduino outputs:
  - 4× DC motor channels via L293D shield
  - 1× PWM signal to HW30A ESC (weapon)
  - 1× PWM signal to self-righting servo

---

## 🛞 Drive System
- **Motors:** 4× TT DC Gear Motors (3–6V)
- **Driver:** L293D Motor Shield
- **Notes:**
  - Motors are run at ~5–6V for safety 🔥
  - Acceleration ramping required in firmware

---

## ⚔️ Weapon System
- **Motor:** RS2205 Brushless (tested on 2S ✅)
- **ESC:** HW30A
- **Control:** Servo-style PWM from Arduino
- **Safety:** Software arming + failsafe required

---

## 🔄 Self-Righting
- **Actuator:** Standard servo
- **Power:** 5V BEC (not shield)

---

## 🧷 Grounding & Protection
- Star ground configuration ⭐
- Bulk capacitor (470–1000µF) on drive rail
- Separate logic power rail required
