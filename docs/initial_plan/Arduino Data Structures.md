# 🧠 Arduino Firmware Data Structures & Enums (Holonomic Bot)

This document defines the **canonical data model** for the firmware.  
All later modules (input, safety, mixer, actuators) consume or modify these structures.

---

## 1️⃣ Channel Enumeration

These indices map **directly** to CRSF/PWM channels.

```cpp
enum ChannelId {
  CH_STRAFE = 0,   // CH1
  CH_FORWARD,     // CH2
  CH_ROTATE,      // CH3
  CH_WEAPON,      // CH4 (LS)
  CH_ARM,         // CH5 (SA)
  CH_KILL,        // CH6 (SE)
  CH_SELF_RIGHT,  // CH7 (SH)
  CH_DRIVE_MODE,  // CH8 (SB)
  CH_COUNT
};
```

---

## 2️⃣ Normalized Input Structure

All analog channels are normalized to **[-1.0 … +1.0]**.

```cpp
struct ControlInput {
  float strafe;      // X
  float forward;     // Y
  float rotate;      // R
  float weapon;      // 0.0 … 1.0
  bool  arm_request;
  bool  kill;
  bool  self_right;
  uint8_t drive_mode; // 0=Beginner,1=Normal,2=Aggressive
};
```

---

## 3️⃣ Safety & Link State

```cpp
enum LinkState {
  LINK_OK,
  LINK_STALE,
  LINK_LOST
};

enum ArmState {
  DISARMED,
  ARMED
};
```

```cpp
struct SafetyState {
  LinkState link;
  ArmState  arm;
  bool      kill_active;
  uint32_t  last_rx_ms;
};
```

---

## 4️⃣ Drive Output Structure

```cpp
struct DriveOutput {
  float fl;
  float fr;
  float rl;
  float rr;
};
```

Values are **[-1.0 … +1.0]** before ramping and clamping.

---

## 5️⃣ Drive Mode Profiles

```cpp
struct DriveProfile {
  float max_duty;
  float accel_ramp;
  float rotate_scale;
};
```

Example table:

```cpp
const DriveProfile DRIVE_PROFILES[3] = {
  {0.45f, 0.015f, 0.6f}, // Beginner 🐢
  {0.65f, 0.025f, 0.8f}, // Normal ⚙️
  {0.85f, 0.040f, 1.0f}  // Aggressive 🐇
};
```

---

## 6️⃣ Global Runtime State

```cpp
struct RuntimeState {
  ControlInput input;
  SafetyState  safety;
  DriveOutput  drive_cmd;
  DriveOutput  drive_actual;
};
```

---

## 7️⃣ Design Notes

- All **modules write to or read from `RuntimeState`**
- No module directly talks to hardware except `actuators`
- Safety logic **overrides** drive and weapon outputs
- This model prevents accidental coupling between subsystems
