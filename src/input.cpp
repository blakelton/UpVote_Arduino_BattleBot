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
  // TODO: Implement telemetry using AlfredoCRSF library if needed
  // Library supports sending telemetry packets via queuePacket() or writePacket()
  // For now, telemetry is disabled to simplify integration
}
