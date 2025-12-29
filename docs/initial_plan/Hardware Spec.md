# 🤖 Battlebot Hardware Specification

**UPGRADED TO 3S** (2025-12-29)

Ref: https://www.printables.com/model/897022-force-knight-1lb-plastic-antweight-combat-robot/files

---

## 🔋 Power Architecture (3S UPGRADE)

### Current System (3S LiPo)
- **Battery:** 3S LiPo (11.1V nominal, 12.6V charged)
- **Weapon Rail (Direct 3S):**
  - Battery ➜ HW30A ESC ➜ RS2205 Brushless Motor ⚔️
  - **3S Performance**: ~50% faster spin, 2.25× kinetic energy
- **Drive Rail (Direct 3S via L293D Shield):**
  - Battery ➜ L293D Motor Shield (11.1V motor power) ➜ 4× TT Motors 🚗
  - **L293D voltage range**: 4.5-36V (3S @11.1V is perfect ✅)
  - **Protection**: 80% duty cycle limit in firmware (8.9V effective max)
- **Logic Rail (Regulated):**
  - Battery ➜ MEIVIFPV BEC (2S-6S → 5V) ➜ Arduino UNO + ELRS Receiver + Servo 🧠
  - **Switching BEC**: Required for 3S (efficient, minimal heat)

⚠️ Logic and motor power are intentionally separated to avoid resets and noise issues.

### Original System (2S LiPo - Historical Reference)
- **Battery:** 2S LiPo (7.4V nominal, 8.4V charged)
- **Drive Rail**: Used buck converter (6.0V output)
- **Upgraded because**: More weapon power needed for competition

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
- **Motors:** 4× TT DC Gear Motors (3–6V rated)
- **Driver:** L293D Motor Shield (4.5-36V input range)
- **3S Operation Notes:**
  - Motors receive PWM-averaged voltage from L293D
  - Firmware limits max duty to 80% (8.9V effective)
  - Monitor L293D chip temperature (<80°C)
  - Acceleration ramping required in firmware

---

## ⚔️ Weapon System
- **Motor:** RS2205 Brushless (2-4S rated, tested on 3S ✅)
- **ESC:** HW30A (2-4S rated)
- **Control:** Servo-style PWM from Arduino
- **3S Performance:** ~50% faster spin speed, 2.25× kinetic energy vs 2S
- **Safety:** Software arming + failsafe required + re-balance weapon for 3S

---

## 🔄 Self-Righting
- **Actuator:** Standard servo
- **Power:** 5V from L293D shield motor terminal (prevents Arduino 5V pin overload)
- **Signal:** Arduino D11 (PWM)

---

## 🧷 Grounding & Protection
- Star ground configuration ⭐
- Bulk capacitor (470–1000µF) on drive rail
- Separate logic power rail required
