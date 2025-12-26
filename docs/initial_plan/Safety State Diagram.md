# 🛑 Safety & Arming State Diagram (Weapon-Critical)

This document defines the **authoritative safety logic** for drive + weapon.

---

## 🧯 Top-Level Safety Priority

Every control loop tick:

1. **KILL active**
2. **Link unhealthy**
3. **ARM invalid**
4. **Weapon throttle not zero**
5. **Normal operation**

Higher priority always overrides lower.

---

## 🔁 State Diagram (Weapon)

```
          ┌──────────────┐
          │   DISARMED   │◄─────────────┐
          └──────┬───────┘              │
                 │ ARM_REQUEST          │
                 │ + throttle≈0         │
                 │ + link OK            │
                 │ + kill inactive      │
                 ▼                      │
          ┌──────────────┐              │
          │    ARMED     │──────────────┘
          └──────┬───────┘
                 │
                 │ any of:
                 │  - kill active
                 │  - link lost
                 │  - ARM switch off
                 │  - reboot
                 ▼
          ┌──────────────┐
          │   DISARMED   │
          └──────────────┘
```

---

## 🔒 Arming Rules (Hard Requirements)

Weapon may transition **DISARMED → ARMED** only if:

- SA is in **middle position**
- Weapon throttle (LS) is within deadband near zero
- Link is healthy
- Kill switch inactive

---

## ❌ Kill Behavior (SE Switch)

| SE Position | Effect |
|------------|-------|
| SE↓ | No override |
| SE– | Immediate disarm + drive stop |
| SE↑ | Same as above (optionally latched) |

Kill overrides **everything**, including ARM.

---

## 📉 Link Loss Behavior

If no valid input frame for >150–200 ms:

- Drive outputs = 0
- Weapon throttle = 0
- Arm state = DISARMED

Re-arm required after recovery.

---

## 🧠 Implementation Notes

- Initialize weapon PWM output **before** any input parsing
- Never cache weapon throttle across disarm events
- Treat invalid frames as link loss

---

## ✅ Acceptance Criteria

- Weapon never spins on boot
- Weapon never spins on reconnect
- Kill switch always wins
- Arming requires deliberate operator action
