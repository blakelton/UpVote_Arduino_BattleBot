# Feature Plan: Phase 2 - Receiver Input Integration (CRSF Protocol)

## Overview

Phase 2 integrates the ELRS receiver via CRSF (Crossfire) protocol over hardware UART, providing normalized control inputs with robust link-loss detection. This phase implements a **custom minimal CRSF parser** optimized for Arduino UNO's limited RAM, focusing exclusively on RC Channels packets (0x16) which contain the 16-channel control data needed for the battlebot.

**Critical insight**: CRSF is a binary protocol running at 420kbaud with CRC validation. Full CRSF libraries are too RAM-heavy for UNO, so we implement only the essential RC Channels packet parsing. Link-loss detection must trigger failsafe within 200ms to meet safety requirements.

## Pre-Phase Requirements

### CRSF Protocol Research
- [ ] Read CRSF protocol specification (TBS Crossfire / ExpressLRS docs)
- [ ] Understand RC Channels packet structure (type 0x16, 22-byte payload)
- [ ] Verify ELRS receiver outputs CRSF format (not SBUS or PWM)
- [ ] Confirm receiver baud rate: 420000 baud

### Hardware Setup
- [ ] **Verify ELRS receiver supports bidirectional CRSF (has RX pin for telemetry)** ⭐ NEW
  - Check receiver datasheet/pinout diagram for RX or "T" (telemetry) pin
  - Common models: EP1/EP2 (yes), SuperP (yes), Nano RX (check variant)
  - If no RX pin available: Can still use RC Channels (RX only), but no telemetry to TX16S
  - **Decision**: Set `CRSF_TELEMETRY_ENABLED` based on receiver capability
- [ ] Connect ELRS receiver TX → Arduino UNO RX (pin 0)
- [ ] **If telemetry desired**: Connect Arduino TX (pin 1) → ELRS receiver RX pin ⭐ NEW
- [ ] Verify receiver powered from 5V BEC (NOT Arduino 5V pin - insufficient current)
- [ ] Test receiver binding with TX16S transmitter
- [ ] Verify receiver outputs packets (logic analyzer or Serial passthrough test)

**Estimated time**: 1-2 hours
**Deliverable**: Verified ELRS receiver outputting CRSF packets

---

## Requirements

### Functional Requirements
- [ ] FR-1: Parse CRSF RC Channels packets (type 0x16) from UART at 420kbaud
- [ ] FR-2: Validate CRC-8-DVB-S2 on all packets; reject invalid frames
- [ ] FR-3: Extract 8 channels from 16-channel payload (11-bit packed format)
- [ ] FR-4: Normalize analog channels to [-1.0, +1.0] range (channels 1-4, 8)
- [ ] FR-5: Decode discrete switches to boolean/enum (channels 5-7)
- [ ] FR-6: Apply deadband (0.05) to analog inputs
- [ ] FR-7: Detect link loss within 200ms of last valid packet
- [ ] FR-8: Update RuntimeState.input structure every valid packet
- [ ] FR-9: Trigger failsafe state on link loss or invalid packet stream
- [ ] **FR-10: Send CRSF telemetry back to TX16S (system stats, error codes, battery voltage)** ⭐ NEW

### Non-Functional Requirements
- [ ] NFR-1: CRSF parsing completes in <2ms per packet
- [ ] NFR-2: SRAM usage <768 bytes total (CRSF buffers + Phase 1)
- [ ] NFR-3: No blocking UART reads in control loop
- [ ] NFR-4: Packet processing rate >50 Hz (CRSF typically sends 100-150Hz)
- [ ] NFR-5: CRC validation 100% accurate (no false accepts)

## Success Criteria
- [ ] SC-1: All 8 channels read correctly, verified via Serial debug print
- [ ] SC-2: Link-loss detected within 200ms (measured with scope on motor outputs)
- [ ] SC-3: 100+ link-loss/recovery cycles without failure
- [ ] SC-4: Deadband prevents stick drift (no creep at center)
- [ ] SC-5: Invalid packets rejected (CRC error injection test)
- [ ] SC-6: Receiver failsafe handled correctly (transmitter off → bot safe within 200ms)

---

## Architectural Analysis

### CRSF Protocol Overview

**Frame Structure**:
```
[DEVICE_ADDR][FRAME_LEN][TYPE][PAYLOAD...][CRC8]
```

**RC Channels Packet (Type 0x16)**:
```
[0xC8][0x18][0x16][22 bytes payload][CRC8]
- ADDR: 0xC8 (flight controller)
- LEN: 0x18 (24 bytes = type + payload + crc)
- TYPE: 0x16 (RC channels)
- PAYLOAD: 16 channels, 11-bit each, packed into 22 bytes
- CRC: CRC-8-DVB-S2 over type + payload
```

**Channel Packing**:
```
16 channels × 11 bits = 176 bits = 22 bytes
Channel values: 0-2047 (11-bit), center = 1024
```

### Design Decisions

| Decision | Rationale | Alternatives Considered |
|----------|-----------|-------------------------|
| Custom minimal parser | UNO has only 2KB RAM; full CRSF libraries too heavy | Use library (rejected: RAM), PWM input (rejected: less robust) |
| Hardware Serial only | CRSF at 420kbaud needs hardware UART; SoftwareSerial unreliable | SoftwareSerial (rejected: can't handle 420kbaud) |
| RC Channels packet only | Only need control channels, not telemetry/link stats | Full CRSF parser (rejected: unnecessary complexity) |
| Packet-level failsafe | Don't trust receiver failsafe; implement in firmware | Receiver failsafe only (rejected: defense in depth) |
| CRC validation required | Reject corrupted packets; treat as link loss | Accept all packets (rejected: unsafe) |

---

## Implementation Plan

### Phase 2.1: CRSF Frame Synchronization & CRC

**Objective**: Detect CRSF frames and validate CRC

#### Tasks

1. **Add to RuntimeState** (`include/state.h`):
   ```cpp
   struct ControlInput {
     float strafe;       // CH1: X axis [-1.0, +1.0]
     float forward;      // CH2: Y axis
     float rotate;       // CH3: R axis
     float weapon;       // CH4: Weapon throttle [0.0, 1.0]
     bool  arm_request;  // CH5: ARM switch (SA middle)
     bool  kill;         // CH6: KILL switch (SE active)
     bool  self_right;   // CH7: Self-right button (SH)
     uint8_t drive_mode; // CH8: 0=Beginner, 1=Normal, 2=Aggressive
   };

   struct RuntimeState {
     // ... existing fields ...
     ControlInput input;
     uint32_t     last_valid_packet_ms;
   };
   ```

2. **Create input module** (`include/input.h`):
   ```cpp
   #ifndef INPUT_H
   #define INPUT_H

   #include <Arduino.h>

   // CRSF constants
   #define CRSF_BAUDRATE        420000
   #define CRSF_FRAME_SIZE_MAX  64
   #define CRSF_PAYLOAD_SIZE_MAX 60
   #define CRSF_ADDRESS_FLIGHT_CONTROLLER 0xC8
   #define CRSF_FRAMETYPE_RC_CHANNELS 0x16
   #define CRSF_RC_CHANNEL_COUNT 16
   #define CRSF_CHANNEL_VALUE_MIN 172    // CRSF min value
   #define CRSF_CHANNEL_VALUE_MID 992    // CRSF center
   #define CRSF_CHANNEL_VALUE_MAX 1811   // CRSF max value

   void input_init();
   void input_update();
   bool input_is_link_healthy();

   #endif
   ```

3. **Implement CRC-8-DVB-S2** (`src/input.cpp`):
   ```cpp
   #include "input.h"
   #include "state.h"
   #include "config.h"

   // CRC-8-DVB-S2 lookup table (polynomial 0xD5)
   static const uint8_t crc8_dvb_s2_table[256] PROGMEM = {
     0x00, 0xD5, 0x7F, 0xAA, 0xFE, 0x2B, 0x81, 0x54,
     // ... (full 256-byte table, store in PROGMEM to save RAM)
   };

   uint8_t crsf_crc8(const uint8_t *data, uint8_t len) {
     uint8_t crc = 0;
     for (uint8_t i = 0; i < len; i++) {
       crc = pgm_read_byte(&crc8_dvb_s2_table[crc ^ data[i]]);
     }
     return crc;
   }
   ```

4. **Frame sync state machine**:
   ```cpp
   static uint8_t frame_buffer[CRSF_FRAME_SIZE_MAX];
   static uint8_t frame_pos = 0;
   static bool sync_found = false;

   void input_init() {
     Serial.begin(CRSF_BAUDRATE);
     frame_pos = 0;
     sync_found = false;
   }

   bool input_read_frame() {
     while (Serial.available()) {
       uint8_t byte = Serial.read();

       if (!sync_found) {
         // Look for address byte
         if (byte == CRSF_ADDRESS_FLIGHT_CONTROLLER) {
           frame_buffer[0] = byte;
           frame_pos = 1;
           sync_found = true;
         }
       } else {
         frame_buffer[frame_pos++] = byte;

         // After 2 bytes, we have addr + len
         if (frame_pos == 2) {
           uint8_t frame_len = frame_buffer[1];
           if (frame_len > CRSF_PAYLOAD_SIZE_MAX + 2) {
             // Invalid length, reset
             sync_found = false;
             frame_pos = 0;
           }
         }

         // Check if frame complete
         if (frame_pos >= 2) {
           uint8_t expected_len = frame_buffer[1] + 2; // +2 for addr+len
           if (frame_pos == expected_len) {
             // Frame complete, validate CRC
             uint8_t crc_calculated = crsf_crc8(&frame_buffer[2], frame_buffer[1] - 1);
             uint8_t crc_received = frame_buffer[expected_len - 1];

             if (crc_calculated == crc_received) {
               // Valid frame!
               sync_found = false;
               frame_pos = 0;
               return true; // Frame ready to process
             } else {
               // CRC fail, reset
               sync_found = false;
               frame_pos = 0;
             }
           }
         }

         // Prevent buffer overflow
         if (frame_pos >= CRSF_FRAME_SIZE_MAX) {
           sync_found = false;
           frame_pos = 0;
         }
       }
     }
     return false; // No complete frame yet
   }
   ```

#### Dependencies
- Requires: Phase 1 complete, ELRS receiver connected
- Enables: Frame-level packet validation

#### Testing Strategy
- **CRC test**: Inject bit flip in packet, verify rejection
- **Sync test**: Send malformed data, verify resynchronization
- **Throughput test**: Verify processes 100+ packets/sec without overflow

#### Acceptance Criteria
- [ ] Frame sync works after power-on
- [ ] CRC validation 100% accurate (test with known good/bad packets)
- [ ] Buffer doesn't overflow under packet flood

---

### Phase 2.2: RC Channels Packet Unpacking

**Objective**: Extract 16 channels from packed 11-bit format

#### Tasks

1. **Implement channel unpacking**:
   ```cpp
   static uint16_t rc_channels[CRSF_RC_CHANNEL_COUNT];

   bool input_parse_rc_channels() {
     // Verify packet type
     if (frame_buffer[2] != CRSF_FRAMETYPE_RC_CHANNELS) {
       return false; // Not RC channels packet
     }

     // Unpack 16 × 11-bit channels from 22 bytes
     // Channels are packed big-endian, 11 bits each
     const uint8_t *payload = &frame_buffer[3];

     // Channel bit unpacking (11 bits per channel, 176 bits total)
     rc_channels[0]  = ((payload[0]    | payload[1]  << 8))                 & 0x07FF;
     rc_channels[1]  = ((payload[1]>>3 | payload[2]  << 5))                 & 0x07FF;
     rc_channels[2]  = ((payload[2]>>6 | payload[3]  << 2 | payload[4]<<10))& 0x07FF;
     rc_channels[3]  = ((payload[4]>>1 | payload[5]  << 7))                 & 0x07FF;
     rc_channels[4]  = ((payload[5]>>4 | payload[6]  << 4))                 & 0x07FF;
     rc_channels[5]  = ((payload[6]>>7 | payload[7]  << 1 | payload[8]<<9)) & 0x07FF;
     rc_channels[6]  = ((payload[8]>>2 | payload[9]  << 6))                 & 0x07FF;
     rc_channels[7]  = ((payload[9]>>5 | payload[10] << 3))                 & 0x07FF;
     rc_channels[8]  = ((payload[11]   | payload[12] << 8))                 & 0x07FF;
     rc_channels[9]  = ((payload[12]>>3| payload[13] << 5))                 & 0x07FF;
     rc_channels[10] = ((payload[13]>>6| payload[14] << 2 | payload[15]<<10))&0x07FF;
     rc_channels[11] = ((payload[15]>>1| payload[16] << 7))                 & 0x07FF;
     rc_channels[12] = ((payload[16]>>4| payload[17] << 4))                 & 0x07FF;
     rc_channels[13] = ((payload[17]>>7| payload[18] << 1 | payload[19]<<9))& 0x07FF;
     rc_channels[14] = ((payload[19]>>2| payload[20] << 6))                 & 0x07FF;
     rc_channels[15] = ((payload[20]>>5| payload[21] << 3))                 & 0x07FF;

     return true;
   }
   ```

#### Dependencies
- Requires: Phase 2.1 complete
- Enables: Raw channel access

#### Testing Strategy
- **Known values test**: Use TX16S with sticks at known positions, verify unpacked values
- **All channels test**: Move each input, verify correct channel changes
- **Bounds test**: Verify all values within 172-1811 range

#### Acceptance Criteria
- [ ] All 16 channels unpack correctly
- [ ] Values match expected from transmitter positions
- [ ] No bit-shift errors (common bug in packing/unpacking)

---

### Phase 2.3: Channel Normalization & Mapping

**Objective**: Convert raw CRSF values to usable control inputs

#### Tasks

1. **Normalization helpers**:
   ```cpp
   float crsf_to_normalized(uint16_t crsf_value) {
     // CRSF range: 172 - 1811, center = 992
     // Map to [-1.0, +1.0]
     float normalized = (float)(crsf_value - CRSF_CHANNEL_VALUE_MID) /
                        (float)(CRSF_CHANNEL_VALUE_MAX - CRSF_CHANNEL_VALUE_MID);
     // Clamp
     if (normalized > 1.0f) normalized = 1.0f;
     if (normalized < -1.0f) normalized = -1.0f;
     return normalized;
   }

   float crsf_to_positive(uint16_t crsf_value) {
     // Map to [0.0, 1.0] for weapon throttle
     float normalized = (float)(crsf_value - CRSF_CHANNEL_VALUE_MIN) /
                        (float)(CRSF_CHANNEL_VALUE_MAX - CRSF_CHANNEL_VALUE_MIN);
     if (normalized > 1.0f) normalized = 1.0f;
     if (normalized < 0.0f) normalized = 0.0f;
     return normalized;
   }

   // 3-position switch decoding
   uint8_t crsf_to_3pos(uint16_t crsf_value) {
     if (crsf_value < 500) return 0;        // Down
     else if (crsf_value < 1500) return 1;  // Middle
     else return 2;                         // Up
   }

   bool crsf_to_bool(uint16_t crsf_value) {
     return (crsf_value > CRSF_CHANNEL_VALUE_MID);
   }
   ```

2. **Apply deadband**:
   ```cpp
   float apply_deadband(float value, float deadband) {
     if (value > -deadband && value < deadband) {
       return 0.0f;
     }
     // Apply linear scaling outside deadband
     if (value > 0.0f) {
       return (value - deadband) / (1.0f - deadband);
     } else {
       return (value + deadband) / (1.0f - deadband);
     }
   }
   ```

3. **Map channels to controls**:
   ```cpp
   void input_update_controls() {
     extern RuntimeState g_state;

     // Analog axes (with deadband)
     float raw_strafe  = crsf_to_normalized(rc_channels[0]);  // CH1
     float raw_forward = crsf_to_normalized(rc_channels[1]);  // CH2
     float raw_rotate  = crsf_to_normalized(rc_channels[2]);  // CH3

     g_state.input.strafe  = apply_deadband(raw_strafe, 0.05f);
     g_state.input.forward = apply_deadband(raw_forward, 0.05f);
     g_state.input.rotate  = apply_deadband(raw_rotate, 0.05f);

     // Weapon throttle (no deadband initially, Phase 5 adds arming deadband)
     g_state.input.weapon = crsf_to_positive(rc_channels[3]);  // CH4

     // Switches
     uint8_t arm_pos = crsf_to_3pos(rc_channels[4]);  // CH5: SA
     g_state.input.arm_request = (arm_pos == 1); // Only middle position arms

     uint8_t kill_pos = crsf_to_3pos(rc_channels[5]); // CH6: SE
     g_state.input.kill = (kill_pos >= 1); // Middle or up = kill

     g_state.input.self_right = crsf_to_bool(rc_channels[6]); // CH7: SH

     g_state.input.drive_mode = crsf_to_3pos(rc_channels[7]); // CH8: SB

     // Update timestamp
     g_state.last_valid_packet_ms = millis();
   }
   ```

#### Dependencies
- Requires: Phase 2.2 complete
- Enables: Usable control inputs

#### Testing Strategy
- **Calibration test**: Center all sticks, verify inputs near 0.0
- **Full range test**: Move sticks to extremes, verify ±1.0
- **Deadband test**: Small stick movements near center, verify 0.0 output
- **Switch test**: Toggle each switch, verify correct state

#### Acceptance Criteria
- [ ] All analog channels normalized correctly
- [ ] Deadband prevents stick drift
- [ ] Switch states decode correctly
- [ ] ARM switch only arms on middle position (safety)

---

### Phase 2.4: Link Health & Failsafe

**Objective**: Detect link loss and trigger failsafe within 200ms

#### Tasks

1. **Update safety module to track link** (`src/safety.cpp`):
   ```cpp
   // Add error codes
   enum SystemError {
     // ... existing ...
     ERR_CRSF_TIMEOUT,      // Link loss
     ERR_CRSF_CRC,          // Too many CRC errors
   };

   bool safety_is_link_ok() {
     extern RuntimeState g_state;
     uint32_t now = millis();
     uint32_t time_since_packet = now - g_state.last_valid_packet_ms;

     // Link is OK if packet received within 200ms
     return (time_since_packet < 200);
   }
   ```

2. **Main update loop** (`src/input.cpp`):
   ```cpp
   void input_update() {
     // Process all available frames
     uint8_t frames_processed = 0;
     while (input_read_frame() && frames_processed < 10) {
       if (input_parse_rc_channels()) {
         input_update_controls();
         frames_processed++;
       }
     }

     // Check link health
     if (!input_is_link_healthy()) {
       safety_set_error(ERR_CRSF_TIMEOUT);
     }
   }

   bool input_is_link_healthy() {
     extern RuntimeState g_state;
     uint32_t now = millis();
     return ((now - g_state.last_valid_packet_ms) < 200);
   }
   ```

3. **Integrate into main loop**:
   ```cpp
   void control_loop_tick() {
     // Read input
     input_update();

     // Check link health
     bool link_ok = safety_is_link_ok();

     if (!link_ok) {
       // FAILSAFE: Force safe outputs
       actuators_set_safe_state();
     } else {
       // Normal operation (Phase 3+ will use inputs)
       actuators_set_safe_state(); // Still safe in Phase 2
     }

     diagnostics_update();
     safety_pet_watchdog();
   }
   ```

#### Dependencies
- Requires: Phase 2.3 complete
- Enables: Safe link-loss handling

#### Testing Strategy
- **Link loss test**: Power off transmitter, verify failsafe within 200ms (scope)
- **Recovery test**: Power transmitter back on, verify inputs resume
- **Range test**: Walk away until link lost, verify safe state
- **Cycle test**: 100+ link loss/recovery cycles

#### Acceptance Criteria
- [ ] Link loss detected within 200ms
- [ ] Failsafe triggers (outputs safe)
- [ ] System recovers cleanly after link restoration
- [ ] No crashes or hangs during link loss

---

### Phase 2.5: CRSF Telemetry (TX16S Display Monitoring) ⭐ NEW

**Objective**: Send system status back to TX16S transmitter display for real-time monitoring

**Why This Matters**: The TX16S can display telemetry data sent from the battlebot. This replaces Serial debugging and provides visibility during matches - you can see error codes, battery voltage, free RAM, and system state on your transmitter screen!

#### CRSF Telemetry Overview

CRSF supports bidirectional communication:
- **RX → Arduino (Pin 0)**: RC Channels packets (already implemented in Phase 2.1-2.4)
- **Arduino → RX (Pin 1 via receiver RX pin)**: Telemetry packets sent back to TX16S

**Telemetry Packet Types** (we'll use):
- **0x08 - Battery Sensor**: Voltage, current, capacity, percent
- **0x14 - Link Statistics**: RSSI, link quality, etc. (optional)
- **Custom Text**: Can send custom messages (advanced)

#### Tasks

1. **Add telemetry configuration** (`include/config.h`):
   ```cpp
   // CRSF Telemetry
   #define CRSF_TELEMETRY_ENABLED  1     // Set to 0 to disable
   #define TELEMETRY_UPDATE_MS     1000  // Send every 1 second

   // Battery monitoring (if available)
   #define BATTERY_MONITOR_PIN     A0    // Analog pin for voltage divider
   #define BATTERY_SCALE_FACTOR    0.0049 // Calibration (2S LiPo, 8.4V max)
   ```

2. **Add telemetry packet builder** (`src/input.cpp`):
   ```cpp
   #if CRSF_TELEMETRY_ENABLED
   static uint32_t last_telemetry_ms = 0;

   // Battery Sensor packet (0x08)
   void crsf_send_battery_telemetry() {
     uint8_t packet[16];
     uint8_t idx = 0;

     // Read battery voltage (if available)
     uint16_t voltage_mv = 0;
     #ifdef BATTERY_MONITOR_PIN
       int adc = analogRead(BATTERY_MONITOR_PIN);
       voltage_mv = (uint16_t)(adc * BATTERY_SCALE_FACTOR * 1000);
     #else
       voltage_mv = 7400; // Placeholder (7.4V nominal for 2S)
     #endif

     // Read free RAM
     int free_ram = diagnostics_get_free_ram();

     // Build packet
     packet[idx++] = 0xC8;           // Device address
     packet[idx++] = 13;             // Frame length (type + payload + crc)
     packet[idx++] = 0x08;           // Type: Battery Sensor

     // Payload (Battery Sensor format):
     // Voltage (uint16_t, big-endian, in 0.1V)
     uint16_t voltage_dv = voltage_mv / 100; // Convert mV to decivolts
     packet[idx++] = (voltage_dv >> 8) & 0xFF;
     packet[idx++] = voltage_dv & 0xFF;

     // Current (uint16_t, big-endian, in 0.1A) - not measured
     packet[idx++] = 0x00;
     packet[idx++] = 0x00;

     // Capacity used (uint24_t, big-endian, in mAh) - not measured
     packet[idx++] = 0x00;
     packet[idx++] = 0x00;
     packet[idx++] = 0x00;

     // Battery remaining (uint8_t, 0-100%) - use free RAM as proxy
     uint8_t ram_percent = (uint8_t)((free_ram * 100) / 2048);
     packet[idx++] = ram_percent;

     // CRC over type + payload
     uint8_t crc = crsf_crc8(&packet[2], idx - 2);
     packet[idx++] = crc;

     // Send packet
     Serial.write(packet, idx);
   }

   void crsf_update_telemetry() {
     uint32_t now = millis();
     if (now - last_telemetry_ms >= TELEMETRY_UPDATE_MS) {
       last_telemetry_ms = now;
       crsf_send_battery_telemetry();
     }
   }
   #endif
   ```

3. **Call telemetry from main loop** (`src/main.cpp`):
   ```cpp
   void control_loop_tick() {
     // Read input
     input_update();

     #if CRSF_TELEMETRY_ENABLED
       crsf_update_telemetry();  // Send status to TX16S
     #endif

     // ... rest of loop ...
   }
   ```

4. **Configure TX16S to display telemetry**:
   - On TX16S: Model Setup → Telemetry → Discover new sensors
   - Should auto-detect "Batt" (battery) sensor
   - Create custom screen to show:
     - Battery voltage (actual voltage if monitored)
     - Battery % (repurposed as free RAM %)
   - View during matches!

#### What You'll See on TX16S

After pairing, your transmitter will show:
```
┌─────────────────┐
│ UPVOTE STATUS   │
├─────────────────┤
│ Batt:  7.4V     │  ← Actual voltage (if monitored)
│ RAM:   87%      │  ← Free RAM (Battery % field)
│ Link:  OK       │  ← Via link quality
└─────────────────┘
```

#### Advanced: Custom Error Codes via Capacity Field

You can repurpose the "Capacity used" field to send error codes:
```cpp
// Instead of 0x00, 0x00, 0x00:
uint32_t error_code = (uint32_t)safety_get_error();
packet[idx++] = (error_code >> 16) & 0xFF;
packet[idx++] = (error_code >> 8) & 0xFF;
packet[idx++] = error_code & 0xFF;
```

Then on TX16S, read "Capacity" to see error codes!

#### Dependencies
- Requires: **Phase 2.1 complete (CRSF CRC-8 function `crsf_crc8()` implemented)** ⭐ CRITICAL
- Requires: Phase 2.4 complete (CRSF RX working, link health monitoring)
- Requires: Diagnostics module with `get_free_ram()` (Phase 1.5)
- Optional: Battery voltage monitor circuit (A0 with voltage divider)
- Enables: Real-time system monitoring on TX16S screen

#### Hardware Note: Connecting Arduino TX to Receiver

**CRITICAL**: The ELRS receiver must support bidirectional CRSF:
- Most ELRS receivers have both TX and RX pins
- Connect **Arduino TX (pin 1)** → **Receiver RX pin**
- This allows Arduino to send telemetry back through the receiver to TX16S

Check your receiver datasheet to identify the RX pin!

#### Testing Strategy
- **Telemetry transmission**: Use logic analyzer to verify packets on pin 1
- **TX16S display**: Verify battery sensor appears after "Discover sensors"
- **Value accuracy**: Compare free RAM on TX16S to build output
- **Update rate**: Verify telemetry updates every 1 second
- **Simultaneous RX/TX stress test** ⭐ CRITICAL:
  - Send telemetry (1 Hz) while receiving RC Channels packets (100-150 Hz)
  - Use logic analyzer to verify NO packet corruption on either RX or TX
  - Verify NO RC Channels packets dropped during telemetry transmission
  - Measure RC Channels latency - must remain <200ms (link check threshold)
  - Run for 5 minutes continuous operation
  - Verify control inputs remain responsive (no stuttering or lag)

#### Acceptance Criteria
- [ ] Telemetry packets sent every 1 second
- [ ] TX16S displays battery voltage (or placeholder)
- [ ] TX16S shows free RAM as battery %
- [ ] Telemetry updates in real-time during operation
- [ ] No impact on RC Channels reception latency (<200ms link check)
- [ ] SRAM usage still within 768-byte Phase 2 budget

#### Memory Impact
- Telemetry packet buffer: 16 bytes (stack allocated, no permanent cost)
- Telemetry state: 4 bytes (last_telemetry_ms)
- **Total overhead: ~20 bytes**

#### Risks & Mitigations
| Risk | Impact | Mitigation |
|------|--------|------------|
| Serial.write() blocks loop | Medium | Limit to 1 Hz; packets are small (~16 bytes) |
| Receiver doesn't support RX pin | Medium | Check datasheet; telemetry is optional |
| TX16S doesn't detect sensor | Low | Use "Discover sensors" menu item |
| Telemetry interferes with RC Channels | Low | Test extensively; CRSF designed for bidirectional |

---

## Testing Strategy

### Unit Testing
- **CRC validation**: Test CRC function with known inputs
- **Channel unpacking**: Verify bit-packing math with manual calculations
- **Normalization**: Test edge cases (min, max, center, deadband boundaries)

### Integration Testing
- **Full packet flow**: Transmitter → receiver → Arduino → RuntimeState
- **Concurrent operation**: Input processing while loop runs at 100Hz
- **Memory usage**: Verify SRAM <768 bytes

### Manual Testing
- **Channel mapping**: All 8 channels respond correctly
- **Link loss response**: <200ms failsafe trigger
- **Error recovery**: Clean recovery from CRC errors, sync loss
- **Transmitter failsafe**: Receiver failsafe outputs handled

---

## Risk Assessment

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| UART buffer overflow at 420kbaud | Medium | High | Process packets frequently; limit frames per loop |
| CRC implementation error | Low | Critical | Test with known CRSF packets; compare to reference |
| Bit-packing errors in channel decode | Medium | High | Test all channels; verify with logic analyzer |
| Deadband too large (unresponsive) | Low | Medium | Make deadband configurable; tune during Phase 4 |
| Link loss detection too slow | Low | Critical | Measure with oscilloscope; optimize if needed |

---

## Open Questions

- [ ] Should we log CRC error rate? (RAM cost vs diagnostics value)
- [ ] Should we support CRSF telemetry response? (Defer to future)
- [ ] Should we validate channel value ranges? (Currently clamped)

---

**Status**: ready
**Created**: 2025-12-25
**Author**: feature-architect (updated with gap analysis mitigations)
**Updates**: Complete CRSF implementation details, custom parser design, memory optimization
