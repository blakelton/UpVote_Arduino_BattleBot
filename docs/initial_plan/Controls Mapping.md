# 🎮 Final Control Map — TX16S → ELRS → Arduino (Holonomic)

## 🕹️ Stick Mapping (unchanged, confirmed)

**Mode 2 assumed**

|Function|Control|
|---|---|
|**Strafe (X)** ↔️|Right stick **Left / Right**|
|**Forward / Back (Y)** ↕️|Right stick **Up / Down**|
|**Rotate (R)** 🔄|Left stick **Left / Right**|

Right stick = planar translation  
Left stick X = yaw  
This is the most intuitive holonomic mapping.

---

## 📡 Channel Assignment (final)

|Channel|Function|TX16S Control|Type|
|---|---|---|---|
|**CH1**|Strafe (X) ↔️|Right stick LR|Analog|
|**CH2**|Forward / Back (Y) ↕️|Right stick UD|Analog|
|**CH3**|Rotate (R) 🔄|Left stick LR|Analog|
|**CH4**|**Weapon Throttle (W)** ⚔️|**LS (Left Slider)**|Analog|
|**CH5**|**ARM switch** 🛑|**SA (3-position)**|Discrete|
|**CH6**|**KILL / PANIC** ❌|**SE (3-position)**|Discrete|
|**CH7**|**Self-Right** 🔄|**SH (momentary)**|Momentary|
|**CH8**|**Drive Mode** 🐢/🐇|**SB (3-position)**|Discrete|

This is an excellent use of the TX16S ergonomics.

---

## 🔘 Switch Semantics (important)

### 🛑 CH5 — ARM (SA, 3-position)

Use this as a **deliberate arming gate**, not a raw boolean.

|SA Position|Meaning|Arduino Interpretation|
|---|---|---|
|**SA↓**|SAFE|`ARM = false`|
|**SA–**|ARM ENABLE|`ARM_REQUEST = true`|
|**SA↑**|RESERVED / FUTURE|Ignored or treated as SAFE|

**Notes**

- Only **SA– (middle)** is allowed to arm.
    
- SA↑ is intentionally ignored for now to prevent accidental arming during switch flicks.
    
- This gives you a _physical “detent”_ for arming — very good practice.
    

---

### ❌ CH6 — KILL / PANIC (SE, 3-position)

This is your **absolute override**.

|SE Position|Meaning|Arduino Action|
|---|---|---|
|**SE↓**|NORMAL|No override|
|**SE–**|SOFT KILL|Drive = 0, Weapon = 0, Disarm|
|**SE↑**|HARD KILL|Same as soft kill + latch until reset or SE↓|

**Notes**

- Treat **SE– and SE↑ identically at first**.
    
- Later you may choose to:
    
    - make SE↑ _latched_ (requires power cycle or explicit reset)
        
    - leave SE– as momentary
        

---

### 🔄 CH7 — Self-Right (SH, momentary)

This is ideal.

|SH State|Action|
|---|---|
|Released|No action|
|Pressed|Activate self-right servo|

**Arduino behavior**

- While SH is held:
    
    - Move servo to “righting” position
        
- On release:
    
    - Return servo to neutral
        
- Servo motion should be **rate-limited** to avoid brownouts.
    

---

### 🐢/🐇 CH8 — Drive Mode (SB, 3-position)

|SB Position|Mode|Typical Use|
|---|---|---|
|**SB↓**|🐢 Beginner|Low max duty, heavy ramp|
|**SB–**|⚙️ Normal|Default tuning|
|**SB↑**|🐇 Aggressive|Higher duty, faster ramp|

**Arduino mapping**

- SB selects a **profile struct**:
    
    - max drive duty
        
    - accel ramp rate
        
    - rotation scale
        
    - optional expo
        

This lets you tune safely _without reflashing_.

---

## ⚔️ CH4 — Weapon Throttle (LS slider)

**Why this is good**

- Sliders are harder to bump accidentally
    
- Excellent resolution for slow ramp-up
    
- Natural mental separation from drive sticks
    

**Arduino rules**

- Weapon throttle ignored unless:
    
    - ARM is valid
        
    - KILL is inactive
        
    - Link is healthy
        
- Deadband near zero required for arming
    

---

## 🧠 Arduino-Side Safety Priority Order (final)

Every control loop tick:

1. **KILL active?**  
    → drive = 0, weapon = 0, disarm
    
2. **Link stale (>150–200 ms)?**  
    → drive = 0, weapon = 0, disarm
    
3. **ARM invalid (SA not middle)?**  
    → weapon = 0, disarm
    
4. **ARM valid but weapon throttle not ~0?**  
    → remain disarmed
    
5. **ARM valid + throttle zero → allow arming**
    
6. Apply drive mixing + ramps
    
7. Apply weapon ramp (if armed)
    

This order is non-negotiable for safety.

---

## 🧮 Holonomic Mixing (confirmed)

Inputs:

- `X = CH1`
    
- `Y = CH2`
    
- `R = CH3`
    

Outputs:

- `FL = Y + X + R`
    
- `FR = Y − X − R`
    
- `RL = Y − X + R`
    
- `RR = Y + X − R`
    

Then:

- Normalize
    
- Apply drive-mode scaling
    
- Apply ramp limiting
    
- Apply final clamps
    

---

## ✅ Why this control map is solid

- ✔️ Physically hard to arm accidentally
    
- ✔️ Explicit kill hierarchy
    
- ✔️ Clear mental model during a match
    
- ✔️ Matches your phased firmware plan
    
- ✔️ Scales cleanly if you change hardware later
    

---

### Next logical step (recommended)

I can generate **one of the following**, fully aligned to this control map:

1. **Arduino data structures & enums** for channels, modes, and states
    
2. **EdgeTX model setup checklist** (step-by-step, no screenshots required)
    
3. **Safety state diagram** (ARM / DISARM / FAILSAFE / KILL) for your docs