# UpVote Battlebot - Documentation Index

**Complete Documentation Suite for Build, Test, and Operation**

Last Updated: 2025-12-26

---

## Quick Navigation

### 🚀 Getting Started

**New to UpVote?** Start here:
1. **[Main README](../README.md)** - Project overview and quick start
2. **[WIRING_DIAGRAM.md](WIRING_DIAGRAM.md)** - Build electrical system
3. **[HARDWARE_TESTING_GUIDE.md](HARDWARE_TESTING_GUIDE.md)** - Test as you build
4. **[OPERATOR_GUIDE.md](OPERATOR_GUIDE.md)** - Learn to operate

### ⚡ Quick Reference (Print These)

For competition day:
- **[COMPETITION_CHECKLIST.md](COMPETITION_CHECKLIST.md)** - Pre/post-match procedures
- **[LED_STATUS_REFERENCE.md](LED_STATUS_REFERENCE.md)** - LED pattern guide
- **[TROUBLESHOOTING.md](TROUBLESHOOTING.md)** - Emergency diagnostics

---

## Complete Documentation List

| Document | Lines | Purpose | Audience | Print? |
|----------|-------|---------|----------|--------|
| **[WIRING_DIAGRAM.md](WIRING_DIAGRAM.md)** | 738 | Complete electrical wiring reference (3S) | Builders | ✅ Yes |
| **[HARDWARE_TESTING_GUIDE.md](HARDWARE_TESTING_GUIDE.md)** | 1,300+ | Incremental testing procedures (12 tests) | Builders | ✅ Yes |
| **[OPERATOR_GUIDE.md](OPERATOR_GUIDE.md)** | 540 | How to operate the bot (3S) | Operators | ✅ Yes |
| **[LED_STATUS_REFERENCE.md](LED_STATUS_REFERENCE.md)** | 407 | LED pattern interpretation | Operators | ✅ Yes |
| **[COMPETITION_CHECKLIST.md](COMPETITION_CHECKLIST.md)** | 573 | Pre-match, in-arena, post-match (3S) | Operators | ✅ Yes |
| **[TROUBLESHOOTING.md](TROUBLESHOOTING.md)** | 580 | Diagnostic procedures (3S) | Operators | ✅ Yes |
| **[3S_UPGRADE_GUIDE.md](3S_UPGRADE_GUIDE.md)** | 1,050+ | Complete 3S upgrade reference | Builders/Operators | ✅ Yes |

**Total Documentation**: 5,200+ lines

---

## Documentation by Use Case

### 🔧 Building the Bot

**Recommended Reading Order**:
1. [WIRING_DIAGRAM.md](WIRING_DIAGRAM.md) - Electrical wiring
   - Component list and power requirements
   - Arduino pinout and connections
   - Wiring checklists
   - Common mistakes and fixes

2. [HARDWARE_TESTING_GUIDE.md](HARDWARE_TESTING_GUIDE.md) - Test as you build
   - 12 incremental tests (2.5-3 hours total)
   - Pass/fail criteria for each test
   - Troubleshooting for each test
   - Test results log template

### 📚 Learning to Operate

**Recommended Reading Order**:
1. [OPERATOR_GUIDE.md](OPERATOR_GUIDE.md) - Operating manual
   - Pre-match setup
   - Power-on sequence
   - Arming procedure
   - Drive controls
   - Emergency procedures

2. [LED_STATUS_REFERENCE.md](LED_STATUS_REFERENCE.md) - LED diagnostics
   - Visual LED patterns
   - Error code reference
   - State transitions

3. [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Problem solving
   - Diagnostic flowcharts
   - Common issues and fixes
   - Hardware fault isolation

### 🏆 Competition Day

**Print These Documents**:
1. [COMPETITION_CHECKLIST.md](COMPETITION_CHECKLIST.md)
   - Pre-event checklist (~40 items, 30-45 min)
   - Pre-match checklist (~25 items, 10-15 min)
   - In-arena procedures
   - Post-match inspection

2. [LED_STATUS_REFERENCE.md](LED_STATUS_REFERENCE.md)
   - Quick LED pattern reference
   - Error codes 1-4

3. [OPERATOR_GUIDE.md](OPERATOR_GUIDE.md)
   - Control mapping
   - Emergency procedures
   - Quick reference card

### 🐛 Troubleshooting

**When Something Goes Wrong**:
1. Check **LED pattern** → [LED_STATUS_REFERENCE.md](LED_STATUS_REFERENCE.md)
2. Find issue in **[TROUBLESHOOTING.md](TROUBLESHOOTING.md)**
3. Verify wiring with **[WIRING_DIAGRAM.md](WIRING_DIAGRAM.md)**
4. Re-test with **[HARDWARE_TESTING_GUIDE.md](HARDWARE_TESTING_GUIDE.md)**

---

## Technical Documentation

### Code Quality Reports

Located in `ai_eval/` subdirectory:
- **FINAL_REPORT.md** - Codebase quality summary
- Component-level reports for each module
- Complexity metrics and analysis

### Firmware Configuration

See main source code:
- **include/config.h** - All system parameters
- **platformio.ini** - Build configuration

---

## Document Summaries

### WIRING_DIAGRAM.md (711 lines)

**What's Inside**:
- System overview (components, power requirements)
- Power distribution diagram
- Arduino UNO pinout with all connections
- Pin assignment table
- Component wiring details (8 sections):
  - Battery to BEC
  - BEC to Arduino
  - BEC to CR8 Nano receiver
  - CR8 to Arduino (CRSF serial)
  - Arduino to 4× drive ESCs
  - Arduino to weapon ESC
  - Arduino to servo
- Wiring checklists (pre-assembly, assembly, verification)
- Common wiring mistakes (7 mistakes with fixes)
- Troubleshooting (6 common issues)
- Optional connections (battery monitoring, external kill switch)
- Wire color standards
- Safety warnings

**Key Sections**:
- Pages 1-3: System overview and power flow
- Pages 4-8: Detailed wiring for each component
- Pages 9-10: Checklists and common mistakes
- Pages 11-12: Troubleshooting and safety

---

### HARDWARE_TESTING_GUIDE.md (1,300+ lines)

**What's Inside**:
- Required equipment checklist
- Test environment setup
- 12 progressive tests:
  1. Arduino Power-On (5 min)
  2. Firmware Upload (10 min)
  3. LED Diagnostics (5 min)
  4. Receiver Power & Binding (10 min)
  5. CRSF Link Verification (10 min)
  6. Telemetry Validation (10 min)
  7. Single Drive Motor (15 min)
  8. Four-Motor Holonomic Drive (20 min)
  9. Weapon Motor Control (15 min)
  10. Self-Right Servo (10 min)
  11. Safety Interlocks (15 min)
  12. Stress Testing (30 min)
- Each test includes:
  - Goal and prerequisites
  - Required equipment
  - Step-by-step procedure
  - Expected results
  - Pass/fail criteria
  - Troubleshooting
  - Notes template
- Test results log template

**Key Features**:
- Incremental validation (each test builds on previous)
- 2.5-3 hours total testing time
- Can spread over multiple sessions
- Documents all failure modes

---

### OPERATOR_GUIDE.md (540 lines)

**What's Inside**:
- Pre-match setup (hardware, transmitter, control checks)
- Power-on sequence (4 steps)
- Arming procedure (weapon arming sequence)
- Drive controls (stick mapping, drive modes)
- Emergency procedures (kill switch, link loss, loop overrun)
- Post-match shutdown (5 steps, battery care)
- LED status indicators (4 patterns + error codes)
- TX16S control mapping (9 channels)
- Quick reference card
- Safety reminders

**Key Sections**:
- Pages 1-4: Pre-match and power-on
- Pages 5-7: Arming and operation
- Pages 8-10: Emergencies and shutdown
- Pages 11-12: LED patterns and controls

---

### LED_STATUS_REFERENCE.md (407 lines)

**What's Inside**:
- Quick reference table (4 patterns)
- Detailed LED patterns with ASCII diagrams:
  - Slow blink (1 Hz) - SAFE
  - Fast blink (5 Hz) - ARMED
  - Solid ON - FAILSAFE
  - Error patterns (1-4 blinks)
- Error code reference (4 codes):
  - 1 blink: Loop Overrun
  - 2 blinks: Watchdog Reset
  - 3 blinks: CRSF Timeout
  - 4 blinks: CRSF CRC Error
- Error details (cause, impact, action, prevention)
- LED state transitions (5 flowcharts)
- Troubleshooting LED issues

**Key Features**:
- Visual ASCII pattern diagrams
- Detailed error explanations
- Recovery procedures for each error

---

### COMPETITION_CHECKLIST.md (573 lines)

**What's Inside**:
- How to use this checklist
- Pre-event checklist (~40 items, 30-45 min):
  - Electronics verification
  - Mechanical inspection
  - Wiring verification
  - Software testing (on blocks)
  - Spares and tools
- Pre-match checklist (~25 items, 10-15 min):
  - Battery check
  - Mechanical inspection
  - Power-on sequence
  - Control verification
- In-arena procedures:
  - Placement
  - Arming sequence (5 steps)
- During match monitoring
- Post-match checklist (5-10 min):
  - Immediate actions
  - Damage assessment
  - Battery management
  - Repairs needed
- Between-matches preparation
- End-of-day shutdown
- Quick reference: LED patterns, safety reminders

**Key Features**:
- Time estimates for each section
- Can be used as physical checklist (print and check boxes)
- Covers entire competition day workflow

---

### TROUBLESHOOTING.md (580 lines)

**What's Inside**:
- LED diagnostic flowchart
- Common issues by category:
  - Power issues (5 scenarios)
  - Link/communication issues (6 scenarios)
  - Motor control issues (8 scenarios)
  - Weapon issues (5 scenarios)
  - Servo issues (4 scenarios)
  - System stability issues (4 scenarios)
- Error code deep-dive (4 error codes)
- Hardware fault isolation procedures
- When to reflash firmware
- Emergency procedures

**Key Sections**:
- Pages 1-3: LED diagnostics and power
- Pages 4-7: Link and motor issues
- Pages 8-10: Weapon, servo, stability
- Pages 11-12: Error codes and hardware faults

---

## Recommended Print Setup

### For Builders (3 documents)
1. **WIRING_DIAGRAM.md** - Reference while building
2. **HARDWARE_TESTING_GUIDE.md** - Testing checklist
3. **OPERATOR_GUIDE.md** - Learn controls

**Paper**: Letter/A4, double-sided
**Binding**: 3-hole punch + binder or stapled
**Pages**: ~60 pages total

### For Operators (2 documents)
1. **OPERATOR_GUIDE.md** - Operating manual
2. **LED_STATUS_REFERENCE.md** - LED diagnostics

**Paper**: Letter/A4, single-sided (easier to reference)
**Binding**: Laminated or plastic sleeve
**Pages**: ~25 pages total

### For Competition (3 documents)
1. **COMPETITION_CHECKLIST.md** - Pre/post-match
2. **LED_STATUS_REFERENCE.md** - Quick LED reference
3. **TROUBLESHOOTING.md** - Emergency diagnostics

**Paper**: Letter/A4, single-sided
**Binding**: Clipboard or laminated
**Pages**: ~40 pages total

---

## Version Information

| Document | Version | Last Updated | Status |
|----------|---------|--------------|--------|
| WIRING_DIAGRAM.md | 1.0 | 2025-12-26 | ✅ Complete |
| HARDWARE_TESTING_GUIDE.md | 1.0 | 2025-12-26 | ✅ Complete |
| OPERATOR_GUIDE.md | 1.0 | 2025-12-26 | ✅ Complete |
| LED_STATUS_REFERENCE.md | 1.0 | 2025-12-26 | ✅ Complete |
| COMPETITION_CHECKLIST.md | 1.0 | 2025-12-26 | ✅ Complete |
| TROUBLESHOOTING.md | 1.0 | 2025-12-26 | ✅ Complete |

**Firmware Version**: Phases 1-6 Complete
**Overall Documentation Status**: ✅ **Production Ready**

---

## Getting Help

### Documentation Issues
If you find errors or have suggestions:
1. Check if issue is hardware-related (see TROUBLESHOOTING.md)
2. Verify wiring (see WIRING_DIAGRAM.md)
3. Re-test systematically (see HARDWARE_TESTING_GUIDE.md)

### During Competition
1. **LED pattern** → LED_STATUS_REFERENCE.md
2. **Specific issue** → TROUBLESHOOTING.md
3. **Pre-match** → COMPETITION_CHECKLIST.md

---

## Additional Resources

### In Repository
- **Main README**: [../README.md](../README.md)
- **AI Evaluation**: [ai_eval/FINAL_REPORT.md](ai_eval/FINAL_REPORT.md)
- **Source Code**: `../include/config.h` for all parameters

### External Links
- **Arduino Reference**: https://docs.arduino.cc
- **ExpressLRS Docs**: https://expresslrs.org
- **EdgeTX Manual**: https://edgetx.org
- **PlatformIO**: https://platformio.org

---

**All documents up to date as of 2025-12-26.**
**Ready for hardware build and testing. 🤖⚡**
