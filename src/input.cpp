// input.cpp - CRSF receiver input implementation (AlfredoCRSF)
// UpVote Battlebot - Phase 7: AlfredoCRSF Integration
#include "input.h"
#include "config.h"
#include "state.h"
#include "safety.h"

// ============================================================================
// ALFREDO CRSF LIBRARY INSTANCE
// ============================================================================

// Global CRSF instance
AlfredoCRSF crsf;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Decode 3-position switch from microseconds
// us < 1200 = position 0 (low)
// 1200 <= us < 1800 = position 1 (mid)
// us >= 1800 = position 2 (high)
static uint8_t decode_3pos_switch_us(uint16_t us) {
  if (us < 1200) return 0;
  if (us < 1800) return 1;
  return 2;
}

// ============================================================================
// INPUT MODULE IMPLEMENTATION
// ============================================================================

void input_init() {
  // Initialize AlfredoCRSF library on Hardware Serial
  // CRSF uses 420000 baud (defined in library as CRSF_BAUDRATE)
  Serial.begin(CRSF_BAUDRATE);
  crsf.begin(Serial);
}

void input_update() {
  // Update CRSF library (reads serial, parses packets)
  crsf.update();

  // Check link status
  g_state.input.link_ok = crsf.isLinkUp();
  if (g_state.input.link_ok) {
    g_state.input.last_packet_ms = millis();
  }

  // Read RC channels (getChannel returns microseconds, 1-based indexing)
  // Expected range: ~988µs (min) to ~2012µs (max), ~1500µs (center)
  uint16_t ch1_us = crsf.getChannel(1);   // Roll (right stick X)
  uint16_t ch2_us = crsf.getChannel(2);   // Pitch (right stick Y)
  uint16_t ch4_us = crsf.getChannel(4);   // Yaw (left stick X)
  uint16_t ch5_us = crsf.getChannel(5);   // Weapon slider
  uint16_t ch6_us = crsf.getChannel(6);   // Arm switch (SA)
  uint16_t ch9_us = crsf.getChannel(9);   // Kill switch (SF)
  uint16_t ch7_us = crsf.getChannel(7);   // Self-right switch (SH)
  uint16_t ch8_us = crsf.getChannel(8);   // Drive mode switch (SB)

  // Convert microseconds to 11-bit CRSF values for integer mixing
  // Library returns ~988-2012µs, CRSF 11-bit is 172-1811
  g_state.input.raw_channels[0] = map(ch1_us, 988, 2012, 172, 1811);  // CH1: Roll
  g_state.input.raw_channels[1] = map(ch2_us, 988, 2012, 172, 1811);  // CH2: Pitch
  g_state.input.raw_channels[3] = map(ch4_us, 988, 2012, 172, 1811);  // CH4: Yaw
  g_state.input.raw_channels[4] = map(ch5_us, 988, 2012, 172, 1811);  // CH5: Weapon
  g_state.input.raw_channels[5] = map(ch6_us, 988, 2012, 172, 1811);  // CH6: Arm
  g_state.input.raw_channels[6] = map(ch7_us, 988, 2012, 172, 1811);  // CH7: Self-right
  g_state.input.raw_channels[7] = map(ch8_us, 988, 2012, 172, 1811);  // CH8: Drive mode
  g_state.input.raw_channels[8] = map(ch9_us, 988, 2012, 172, 1811);  // CH9: Kill

  // Decode switches (3-position)
  uint8_t arm_switch_pos = decode_3pos_switch_us(ch6_us);       // CH6: Arm Switch (SA)
  uint8_t kill_switch_pos = decode_3pos_switch_us(ch9_us);      // CH9: Kill Switch (SF)
  uint8_t selfright_switch_pos = decode_3pos_switch_us(ch7_us); // CH7: Self-Right (SH)

  // Map switch positions to boolean states
  // Arm switch: position 2 (high) = armed
  g_state.input.arm_switch = (arm_switch_pos == 2);

  // Kill switch: position 2 (high) = kill active
  g_state.input.kill_switch = (kill_switch_pos == 2);

  // Self-right switch: position 2 (high) = trigger
  g_state.input.selfright_switch = (selfright_switch_pos == 2);

  // Weapon throttle: map from microseconds to 0.0-1.0 range
  // Weapon uses unipolar control (0-100%), not bipolar (-100 to +100%)
  g_state.input.weapon = constrain(map(ch5_us, 988, 2012, 0, 100), 0, 100) / 100.0f;
}

void input_update_telemetry() {
  // Rate limit telemetry updates (1 Hz by default)
  uint32_t now = millis();
  if (now - g_state.battery.last_telemetry_ms < TELEMETRY_UPDATE_MS) {
    return;
  }
  g_state.battery.last_telemetry_ms = now;

#if CRSF_TELEMETRY_ENABLED
  // ========================================================================
  // STEP 1: Read battery voltage from ADC
  // ========================================================================
  // Voltage divider: R1=10k, R2=3.3k
  // Vout = Vin * R2/(R1+R2), so Vin = Vout * (R1+R2)/R2
  // ADC gives 0-1023 for 0-5V reference
  int adc_raw = analogRead(PIN_BATTERY_MONITOR);
  float v_adc = (float)adc_raw * BATTERY_ADC_VREF / (float)BATTERY_ADC_MAX;
  float v_battery = v_adc * BATTERY_DIVIDER_RATIO;

  // Store in state for other modules to access
  g_state.battery.voltage = v_battery;

  // ========================================================================
  // STEP 2: Calculate battery percentage
  // ========================================================================
  // Linear mapping from min to max voltage
  float pct = (v_battery - BATTERY_VOLTAGE_MIN) / (BATTERY_VOLTAGE_MAX - BATTERY_VOLTAGE_MIN) * 100.0f;
  pct = constrain(pct, 0.0f, 100.0f);
  g_state.battery.percentage = (uint8_t)pct;

  // ========================================================================
  // STEP 3: Build CRSF battery sensor telemetry packet
  // ========================================================================
  // CRSF battery sensor payload (8 bytes):
  // - voltage: uint16_t, V * 10, big endian
  // - current: uint16_t, A * 10, big endian
  // - capacity: uint24_t (3 bytes), mAh, big endian
  // - remaining: uint8_t, percentage

  // Convert voltage to CRSF format (V * 10)
  // Example: 11.5V -> 115 (0x0073)
  // Big endian: high byte first -> [0x00][0x73]
  uint16_t voltage_raw = (uint16_t)(v_battery * 10.0f);

  // Build the payload manually - CRSF uses big endian (MSB first)
  // Payload layout: [voltage_hi][voltage_lo][current_hi][current_lo][cap_hi][cap_mid][cap_lo][remaining]
  uint8_t payload[8];
  payload[0] = (voltage_raw >> 8) & 0xFF;   // voltage high byte
  payload[1] = voltage_raw & 0xFF;           // voltage low byte
  payload[2] = 0;                            // current high byte (not measured)
  payload[3] = 0;                            // current low byte
  payload[4] = 0;                            // capacity high byte
  payload[5] = 0;                            // capacity mid byte
  payload[6] = 0;                            // capacity low byte
  payload[7] = g_state.battery.percentage;   // remaining percentage

  // ========================================================================
  // STEP 4: Send telemetry packet via CRSF
  // ========================================================================
  // Use queuePacket to avoid blocking - it buffers the packet for transmission
  crsf.queuePacket(
    CRSF_ADDRESS_FLIGHT_CONTROLLER,  // Source address (we are the FC)
    CRSF_FRAMETYPE_BATTERY_SENSOR,   // Frame type 0x08
    payload,                          // Payload data
    sizeof(payload)                   // Payload length (8 bytes)
  );
#endif // CRSF_TELEMETRY_ENABLED
}
