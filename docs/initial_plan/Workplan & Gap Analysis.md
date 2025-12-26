# 🛠️ Battlebot Workplan & Gap Analysis

## 📍 Current Progress
- ✅ Drive motors selected (TT motors)
- ✅ Weapon motor tested on 2S (RS2205 + HW30A)
- ✅ Control platform chosen (Arduino UNO)
- ✅ Motor shield selected (L293D)
- ✅ Control link chosen (TX16S + ELRS)

---

## 🧭 Current Plan
1. Use **2S LiPo** as single energy source 🔋
2. Split power into:
   - Raw 2S for weapon ⚔️
   - 6V regulated for drive 🚗
   - 5V regulated for logic 🧠
3. Arduino handles:
   - Drive motor mixing
   - Weapon arming + throttle
   - Failsafe timeout ⏱️
4. L293D shield used for **prototype-level drive control**
5. HW30A ESC controlled via PWM for weapon

---

## ⚠️ Gaps & Risks Remaining
### 1️⃣ L293D Current Limits
- Risk of overheating under stall conditions 🔥
- Mitigation:
  - Voltage limited to 5–6V
  - Acceleration ramping
  - Output duty clamp (≤80%)

### 2️⃣ Power Noise & Resets
- Solved via:
  - Separate 5V logic BEC
  - Star ground layout ⭐
  - Bulk capacitor on drive rail

### 3️⃣ Safety & Failsafe
- Must implement in firmware:
  - Link-loss timeout → motors OFF ❌
  - Weapon requires ARM switch 🛑
  - Default disarmed on boot

### 4️⃣ ELRS Integration
- CRSF parsing on Arduino UNO UART
- USB debug must not interfere during operation

---

## 🔜 Next Actions
1. Install buck converters (6V + 5V)
2. Wire star-grounded power rails
3. Add bulk capacitor
4. Write and test Arduino firmware 🧪
5. Bench test before combat ⚠️
