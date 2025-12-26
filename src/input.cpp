// input.cpp - CRSF receiver input implementation
// UpVote Battlebot - Phase 2
#include "input.h"
#include "config.h"
#include "state.h"
#include "safety.h"
#include "diagnostics.h"
#include <avr/pgmspace.h>

// ============================================================================
// PHASE 2.2: CRC-8-DVB-S2 LOOKUP TABLE (stored in flash to save RAM)
// ============================================================================

// CRC-8-DVB-S2 polynomial: 0xD5
// Lookup table for fast CRC calculation (256 bytes in PROGMEM)
static const uint8_t crc8_dvb_s2_table[256] PROGMEM = {
  0x00, 0xD5, 0x7F, 0xAA, 0xFE, 0x2B, 0x81, 0x54, 0x29, 0xFC, 0x56, 0x83, 0xD7, 0x02, 0xA8, 0x7D,
  0x52, 0x87, 0x2D, 0xF8, 0xAC, 0x79, 0xD3, 0x06, 0x7B, 0xAE, 0x04, 0xD1, 0x85, 0x50, 0xFA, 0x2F,
  0xA4, 0x71, 0xDB, 0x0E, 0x5A, 0x8F, 0x25, 0xF0, 0x8D, 0x58, 0xF2, 0x27, 0x73, 0xA6, 0x0C, 0xD9,
  0xF6, 0x23, 0x89, 0x5C, 0x08, 0xDD, 0x77, 0xA2, 0xDF, 0x0A, 0xA0, 0x75, 0x21, 0xF4, 0x5E, 0x8B,
  0x9D, 0x48, 0xE2, 0x37, 0x63, 0xB6, 0x1C, 0xC9, 0xB4, 0x61, 0xCB, 0x1E, 0x4A, 0x9F, 0x35, 0xE0,
  0xCF, 0x1A, 0xB0, 0x65, 0x31, 0xE4, 0x4E, 0x9B, 0xE6, 0x33, 0x99, 0x4C, 0x18, 0xCD, 0x67, 0xB2,
  0x39, 0xEC, 0x46, 0x93, 0xC7, 0x12, 0xB8, 0x6D, 0x10, 0xC5, 0x6F, 0xBA, 0xEE, 0x3B, 0x91, 0x44,
  0x6B, 0xBE, 0x14, 0xC1, 0x95, 0x40, 0xEA, 0x3F, 0x42, 0x97, 0x3D, 0xE8, 0xBC, 0x69, 0xC3, 0x16,
  0xEF, 0x3A, 0x90, 0x45, 0x11, 0xC4, 0x6E, 0xBB, 0xC6, 0x13, 0xB9, 0x6C, 0x38, 0xED, 0x47, 0x92,
  0xBD, 0x68, 0xC2, 0x17, 0x43, 0x96, 0x3C, 0xE9, 0x94, 0x41, 0xEB, 0x3E, 0x6A, 0xBF, 0x15, 0xC0,
  0x4B, 0x9E, 0x34, 0xE1, 0xB5, 0x60, 0xCA, 0x1F, 0x62, 0xB7, 0x1D, 0xC8, 0x9C, 0x49, 0xE3, 0x36,
  0x19, 0xCC, 0x66, 0xB3, 0xE7, 0x32, 0x98, 0x4D, 0x30, 0xE5, 0x4F, 0x9A, 0xCE, 0x1B, 0xB1, 0x64,
  0x72, 0xA7, 0x0D, 0xD8, 0x8C, 0x59, 0xF3, 0x26, 0x5B, 0x8E, 0x24, 0xF1, 0xA5, 0x70, 0xDA, 0x0F,
  0x20, 0xF5, 0x5F, 0x8A, 0xDE, 0x0B, 0xA1, 0x74, 0x09, 0xDC, 0x76, 0xA3, 0xF7, 0x22, 0x88, 0x5D,
  0xD6, 0x03, 0xA9, 0x7C, 0x28, 0xFD, 0x57, 0x82, 0xFF, 0x2A, 0x80, 0x55, 0x01, 0xD4, 0x7E, 0xAB,
  0x84, 0x51, 0xFB, 0x2E, 0x7A, 0xAF, 0x05, 0xD0, 0xAD, 0x78, 0xD2, 0x07, 0x53, 0x86, 0x2C, 0xF9
};

// Calculate CRC-8-DVB-S2 for a buffer
// Initial CRC = 0x00, polynomial = 0xD5
static uint8_t crsf_crc8(const uint8_t *data, uint8_t len) {
  uint8_t crc = 0;
  for (uint8_t i = 0; i < len; i++) {
    crc = pgm_read_byte(&crc8_dvb_s2_table[crc ^ data[i]]);
  }
  return crc;
}

// ============================================================================
// PHASE 2.3: 11-BIT CHANNEL UNPACKING
// ============================================================================

// Extract 16 channels from 22-byte packed payload
// CRSF channels are 11-bit values (0-2047) packed into 22 bytes
// Channel layout: 16 channels × 11 bits = 176 bits = 22 bytes
static void crsf_unpack_channels(const uint8_t *payload, uint16_t *channels) {
  // Payload contains 16 channels packed as 11-bit values
  // We need to extract each 11-bit value from the bit stream

  // Extract all 16 channels
  // Each channel is 11 bits, starting from bit 0
  channels[0]  = ((payload[0]    | payload[1]  << 8)                    ) & 0x07FF;  // 11 bits
  channels[1]  = ((payload[1]>>3 | payload[2]  << 5)                    ) & 0x07FF;
  channels[2]  = ((payload[2]>>6 | payload[3]  << 2 | payload[4] << 10)) & 0x07FF;
  channels[3]  = ((payload[4]>>1 | payload[5]  << 7)                    ) & 0x07FF;
  channels[4]  = ((payload[5]>>4 | payload[6]  << 4)                    ) & 0x07FF;
  channels[5]  = ((payload[6]>>7 | payload[7]  << 1 | payload[8] << 9 )) & 0x07FF;
  channels[6]  = ((payload[8]>>2 | payload[9]  << 6)                    ) & 0x07FF;
  channels[7]  = ((payload[9]>>5 | payload[10] << 3)                    ) & 0x07FF;
  channels[8]  = ((payload[11]   | payload[12] << 8)                    ) & 0x07FF;
  channels[9]  = ((payload[12]>>3| payload[13] << 5)                    ) & 0x07FF;
  channels[10] = ((payload[13]>>6| payload[14] << 2 | payload[15] << 10)) & 0x07FF;
  channels[11] = ((payload[15]>>1| payload[16] << 7)                    ) & 0x07FF;
  channels[12] = ((payload[16]>>4| payload[17] << 4)                    ) & 0x07FF;
  channels[13] = ((payload[17]>>7| payload[18] << 1 | payload[19] << 9 )) & 0x07FF;
  channels[14] = ((payload[19]>>2| payload[20] << 6)                    ) & 0x07FF;
  channels[15] = ((payload[20]>>5| payload[21] << 3)                    ) & 0x07FF;
}

// ============================================================================
// PHASE 2.4: CHANNEL NORMALIZATION + DEADBAND
// ============================================================================

// Normalize CRSF channel value (11-bit 0-2047) to float range
// raw: Raw CRSF channel value (0-2047)
// Returns: Normalized value (-1.0 to +1.0)
static float normalize_channel(uint16_t raw) {
  // CRSF range: 172 (min) to 1811 (max), center at 992
  // Map to -1.0 ... +1.0 range
  float normalized = (float)(raw - CRSF_CHANNEL_VALUE_MID) /
                     (float)(CRSF_CHANNEL_VALUE_MAX - CRSF_CHANNEL_VALUE_MID);

  // Clamp to [-1.0, +1.0]
  if (normalized < -1.0f) normalized = -1.0f;
  if (normalized > +1.0f) normalized = +1.0f;

  return normalized;
}

// Apply deadband filter to normalized input
// input: Normalized input value (-1.0 to +1.0)
// deadband: Deadband threshold (e.g., 0.05 for 5%)
// Returns: Filtered value with deadband applied
static float apply_deadband(float input, float deadband) {
  if (input >= -deadband && input <= deadband) {
    // Inside deadband zone → return zero
    return 0.0f;
  }

  // Outside deadband → scale to full range
  // This removes the deadband gap and re-scales to full -1.0 ... +1.0
  if (input > 0.0f) {
    return (input - deadband) / (1.0f - deadband);
  } else {
    return (input + deadband) / (1.0f - deadband);
  }
}

// Decode 3-position switch from CRSF channel
// raw: Raw CRSF channel value
// Returns: Switch position (0, 1, or 2)
static uint8_t decode_3pos_switch(uint16_t raw) {
  // CRSF 3-position switch encoding:
  // Low:    172-700   → position 0
  // Middle: 701-1300  → position 1
  // High:   1301-1811 → position 2
  if (raw < 700) return 0;
  if (raw < 1300) return 1;
  return 2;
}

// ============================================================================
// PRIVATE STATE - CRSF Frame Parsing
// ============================================================================

// Frame synchronization state
enum CRSFSyncState {
  WAITING_FOR_ADDRESS,  // Looking for 0xC8 address byte
  WAITING_FOR_LENGTH,   // Reading frame length
  WAITING_FOR_TYPE,     // Reading frame type
  READING_PAYLOAD,      // Reading payload bytes
  READING_CRC           // Reading CRC byte
};

// CRSF parser state
static struct {
  CRSFSyncState sync_state;      // Current sync state
  uint8_t frame_buffer[CRSF_FRAME_SIZE_MAX];  // Frame assembly buffer
  uint8_t frame_length;          // Expected frame length
  uint8_t bytes_received;        // Bytes received in current frame
  uint16_t channels_raw[16];     // Raw 11-bit channel values (0-2047)
} crsf_parser;

// ============================================================================
// PHASE 2.1: UART INITIALIZATION + FRAME SYNCHRONIZATION
// ============================================================================

void input_init() {
  // Initialize Hardware Serial for CRSF
  // CR8 Nano TX → Arduino RX (pin 0)
  // Arduino TX (pin 1) → CR8 Nano RX (for Phase 2.5 telemetry)
  Serial.begin(CRSF_BAUDRATE);

  // Initialize parser state
  crsf_parser.sync_state = WAITING_FOR_ADDRESS;
  crsf_parser.frame_length = 0;
  crsf_parser.bytes_received = 0;

  // Initialize input state to safe defaults
  g_state.input.roll = 0.0f;
  g_state.input.pitch = 0.0f;
  g_state.input.yaw = 0.0f;
  g_state.input.throttle = 0.0f;
  g_state.input.weapon = 0.0f;
  g_state.input.arm_switch = false;
  g_state.input.kill_switch = false;
  g_state.input.selfright_switch = false;
  g_state.input.last_packet_ms = millis();
  g_state.input.link_ok = false;
}

// ============================================================================
// FRAME SYNCHRONIZATION STATE HANDLERS
// ============================================================================

// Process address byte in frame synchronization
// byte: Incoming byte to check for flight controller address
static void handle_address_byte(uint8_t byte) {
  // Looking for flight controller address (0xC8)
  if (byte == CRSF_ADDRESS_FLIGHT_CONTROLLER) {
    crsf_parser.frame_buffer[0] = byte;
    crsf_parser.bytes_received = 1;
    crsf_parser.sync_state = WAITING_FOR_LENGTH;
  }
  // Ignore other bytes (sync loss, noise, etc.)
}

// Process length byte in frame synchronization
// byte: Frame length byte (excludes address, includes type + payload + CRC)
static void handle_length_byte(uint8_t byte) {
  crsf_parser.frame_length = byte;

  // Sanity check: length must be valid
  if (crsf_parser.frame_length > 0 &&
      crsf_parser.frame_length <= CRSF_PAYLOAD_SIZE_MAX) {
    crsf_parser.frame_buffer[1] = byte;
    crsf_parser.bytes_received = 2;
    crsf_parser.sync_state = WAITING_FOR_TYPE;
  } else {
    // Invalid length → resync
    crsf_parser.sync_state = WAITING_FOR_ADDRESS;
  }
}

// Process type byte in frame synchronization
// byte: Frame type byte
static void handle_type_byte(uint8_t byte) {
  crsf_parser.frame_buffer[2] = byte;
  crsf_parser.bytes_received = 3;

  // If payload expected, read it; otherwise go straight to CRC
  if (crsf_parser.frame_length > 2) {
    crsf_parser.sync_state = READING_PAYLOAD;
  } else {
    crsf_parser.sync_state = READING_CRC;
  }
}

// Process payload byte in frame synchronization
// byte: Payload data byte
static void handle_payload_byte(uint8_t byte) {
  // Reading payload bytes
  crsf_parser.frame_buffer[crsf_parser.bytes_received] = byte;
  crsf_parser.bytes_received++;

  // Check if we've read: type (1) + payload + CRC position
  // frame_length = type + payload + CRC
  // We've read 2 bytes (addr, len) + frame_length bytes total
  if (crsf_parser.bytes_received >= 2 + crsf_parser.frame_length - 1) {
    crsf_parser.sync_state = READING_CRC;
  }
}

// Process RC Channels frame payload
// payload: Pointer to 22-byte RC channels payload
static void process_rc_channels_frame(const uint8_t *payload) {
  // Unpack 16 channels from 22-byte payload
  crsf_unpack_channels(payload, crsf_parser.channels_raw);

  // Phase 2.4: Normalize and apply deadband
  // TX16S Channel mapping (from docs/initial_plan/control_mapping.md):
  // CH1: Roll (right stick X)
  // CH2: Pitch (right stick Y)
  // CH3: Throttle (left stick Y) - unused for holonomic
  // CH4: Yaw (left stick X)
  // CH5: Arm switch (SA/SB/SC 3-pos)
  // CH6: Kill switch (SD 2-pos)
  // CH7: Self-right switch (SE/SF)
  // CH8: Weapon throttle (slider/pot)

  // Analog sticks (normalize + deadband)
  float roll_raw = normalize_channel(crsf_parser.channels_raw[0]);
  float pitch_raw = normalize_channel(crsf_parser.channels_raw[1]);
  float yaw_raw = normalize_channel(crsf_parser.channels_raw[3]);
  float weapon_raw = normalize_channel(crsf_parser.channels_raw[7]);

  g_state.input.roll = apply_deadband(roll_raw, INPUT_DEADBAND);
  g_state.input.pitch = apply_deadband(pitch_raw, INPUT_DEADBAND);
  g_state.input.yaw = apply_deadband(yaw_raw, INPUT_DEADBAND);

  // Weapon throttle: 0.0 to +1.0 (no deadband, no negative)
  // Map from -1.0...+1.0 to 0.0...+1.0
  g_state.input.weapon = (weapon_raw + 1.0f) / 2.0f;
  if (g_state.input.weapon < 0.0f) g_state.input.weapon = 0.0f;
  if (g_state.input.weapon > 1.0f) g_state.input.weapon = 1.0f;

  // Throttle (unused for holonomic drive, but read anyway)
  g_state.input.throttle = apply_deadband(normalize_channel(crsf_parser.channels_raw[2]), INPUT_DEADBAND);

  // Switches (decode positions)
  uint8_t arm_switch_pos = decode_3pos_switch(crsf_parser.channels_raw[4]);
  uint8_t kill_switch_pos = decode_3pos_switch(crsf_parser.channels_raw[5]);
  uint8_t selfright_switch_pos = decode_3pos_switch(crsf_parser.channels_raw[6]);

  // Arm switch: position 2 (high) = armed
  g_state.input.arm_switch = (arm_switch_pos == 2);

  // Kill switch: position 2 (high) = kill active
  g_state.input.kill_switch = (kill_switch_pos == 2);

  // Self-right switch: position 2 (high) = trigger
  g_state.input.selfright_switch = (selfright_switch_pos == 2);
}

// Process CRC byte and validate complete frame
// byte: CRC byte
static void handle_crc_byte(uint8_t byte) {
  // Final CRC byte
  crsf_parser.frame_buffer[crsf_parser.bytes_received] = byte;
  crsf_parser.bytes_received++;

  // Complete frame received
  // Frame structure in buffer:
  // [0] = address (0xC8)
  // [1] = length
  // [2] = type
  // [3..N-1] = payload
  // [N] = CRC

  // Phase 2.2: Validate CRC-8-DVB-S2
  // CRC is calculated over type + payload (excludes address and length)
  // CRC start: frame_buffer[2] (type byte)
  // CRC length: frame_length - 1 (excludes CRC byte itself)
  uint8_t crc_start_index = 2;
  uint8_t crc_length = crsf_parser.frame_length - 1;
  uint8_t calculated_crc = crsf_crc8(&crsf_parser.frame_buffer[crc_start_index], crc_length);
  uint8_t received_crc = crsf_parser.frame_buffer[crsf_parser.bytes_received - 1];

  if (calculated_crc == received_crc) {
    // CRC valid - process frame based on type
    uint8_t frame_type = crsf_parser.frame_buffer[2];

    if (frame_type == CRSF_FRAMETYPE_RC_CHANNELS) {
      // Phase 2.3: Parse RC Channels packet
      // Payload starts at frame_buffer[3]
      // Payload is 22 bytes for RC Channels
      // Validate payload length: frame_length = type (1) + payload (22) + CRC (1) = 24
      uint8_t expected_frame_length = 1 + CRSF_RC_CHANNELS_PAYLOAD_SIZE + 1;

      if (crsf_parser.frame_length == expected_frame_length) {
        const uint8_t *payload = &crsf_parser.frame_buffer[3];
        process_rc_channels_frame(payload);
      }
      // Invalid payload length - silently ignore (malformed packet)
    }

    // Update link status (got valid packet)
    g_state.input.last_packet_ms = millis();
    g_state.input.link_ok = true;
  } else {
    // CRC mismatch - reject frame
    // Treat as link issue (could be noise, interference, etc.)
    safety_set_error(ERR_CRSF_CRC);
  }

  // Reset state machine for next frame
  crsf_parser.sync_state = WAITING_FOR_ADDRESS;
  crsf_parser.bytes_received = 0;
}

// Frame synchronization state machine
// Processes one byte at a time, assembles complete CRSF frames
static void crsf_process_byte(uint8_t byte) {
  switch (crsf_parser.sync_state) {
    case WAITING_FOR_ADDRESS:
      handle_address_byte(byte);
      break;

    case WAITING_FOR_LENGTH:
      handle_length_byte(byte);
      break;

    case WAITING_FOR_TYPE:
      handle_type_byte(byte);
      break;

    case READING_PAYLOAD:
      handle_payload_byte(byte);
      break;

    case READING_CRC:
      handle_crc_byte(byte);
      break;
  }
}

// ============================================================================
// Phase 2.5: CRSF Telemetry (TX to Receiver)
// ============================================================================

#if CRSF_TELEMETRY_ENABLED

// Track last telemetry transmission time
static uint32_t last_telemetry_ms = 0;

// Send battery telemetry packet (0x08)
// Repurposes fields for battlebot status:
// - Voltage: Battery voltage (if monitored) or nominal 7.4V
// - Current: Not used (0)
// - Capacity: Error code (low 16 bits)
// - Remaining %: Free RAM percentage
static void crsf_send_battery_telemetry() {
  uint8_t packet[16];
  uint8_t idx = 0;

  // Build packet header
  packet[idx++] = CRSF_ADDRESS_FLIGHT_CONTROLLER;  // 0xC8
  packet[idx++] = 11;  // Frame length (type + 8 payload + CRC)
  packet[idx++] = CRSF_FRAMETYPE_BATTERY_SENSOR;   // 0x08

  // Payload: Battery Sensor format (8 bytes)

  // Voltage (uint16_t, big-endian, in decivolts 0.1V)
  uint16_t voltage_dv = 0;
  #ifdef BATTERY_MONITOR_PIN
    // Read battery voltage from ADC
    int adc = analogRead(BATTERY_MONITOR_PIN);
    float voltage = adc * BATTERY_SCALE_FACTOR;
    voltage_dv = (uint16_t)(voltage * 10.0f);  // Convert to decivolts
  #else
    // No battery monitoring - report nominal 2S LiPo voltage (7.4V)
    voltage_dv = 74;  // 7.4V in decivolts
  #endif
  packet[idx++] = (voltage_dv >> 8) & 0xFF;  // High byte
  packet[idx++] = voltage_dv & 0xFF;         // Low byte

  // Current (uint16_t, big-endian, in deciamps 0.1A) - not measured
  packet[idx++] = 0x00;
  packet[idx++] = 0x00;

  // Capacity used (uint24_t, big-endian, in mAh)
  // Repurpose: Send error code in low 16 bits
  uint16_t error_code = (uint16_t)g_state.safety.error;
  packet[idx++] = 0x00;                      // High byte (always 0)
  packet[idx++] = (error_code >> 8) & 0xFF;  // Error code high byte
  packet[idx++] = error_code & 0xFF;         // Error code low byte

  // Battery remaining (uint8_t, 0-100%)
  // Repurpose: Send free RAM percentage
  int free_ram = diagnostics_get_free_ram();
  uint8_t ram_percent = (uint8_t)((free_ram * 100L) / 2048L);
  if (ram_percent > 100) ram_percent = 100;  // Clamp to 100%
  packet[idx++] = ram_percent;

  // Calculate CRC over type + payload (excludes address and length)
  uint8_t crc = crsf_crc8(&packet[2], idx - 2);
  packet[idx++] = crc;

  // Send packet to receiver
  Serial.write(packet, idx);
}

#endif  // CRSF_TELEMETRY_ENABLED

// ============================================================================
// Input Update Functions
// ============================================================================

void input_update() {
  // Process all available UART bytes (non-blocking)
  while (Serial.available() > 0) {
    uint8_t byte = Serial.read();
    crsf_process_byte(byte);
  }

  // Phase 2.4: Link health monitoring
  // Check for link timeout (no valid packet in LINK_TIMEOUT_MS)
  uint32_t time_since_last_packet = millis() - g_state.input.last_packet_ms;

  if (time_since_last_packet > LINK_TIMEOUT_MS) {
    // Link lost - set failsafe
    g_state.input.link_ok = false;
    safety_set_error(ERR_CRSF_TIMEOUT);

    // Reset inputs to safe values
    g_state.input.roll = 0.0f;
    g_state.input.pitch = 0.0f;
    g_state.input.yaw = 0.0f;
    g_state.input.throttle = 0.0f;
    g_state.input.weapon = 0.0f;
    g_state.input.arm_switch = false;
    g_state.input.kill_switch = false;
    g_state.input.selfright_switch = false;
  }
}

void input_update_telemetry() {
  #if CRSF_TELEMETRY_ENABLED
    // Rate-limit telemetry to 1 Hz (every 1000ms)
    uint32_t now = millis();
    if (now - last_telemetry_ms >= TELEMETRY_UPDATE_MS) {
      last_telemetry_ms = now;
      crsf_send_battery_telemetry();
    }
  #endif
}
