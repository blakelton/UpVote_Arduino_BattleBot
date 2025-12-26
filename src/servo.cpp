// servo.cpp - Self-righting servo control
// UpVote Battlebot - Phase 6
#include "servo.h"
#include "config.h"
#include "state.h"

// ============================================================================
// PRIVATE STATE
// ============================================================================

// Previous servo command (for slew-rate limiting)
static uint16_t g_servo_previous_us = SERVO_NEUTRAL_US;

// ============================================================================
// PRIVATE HELPER FUNCTIONS
// ============================================================================

// Calculate servo output with rate limiting and endpoint clamping
// Returns pulse width in microseconds [SERVO_ENDPOINT_RETRACT, SERVO_ENDPOINT_EXTEND]
static uint16_t servo_calculate_output() {
  uint16_t target_us = SERVO_NEUTRAL_US;

  // Check if self-right button is pressed AND link is healthy
  // Failsafe: If link lost or kill switch active, return to neutral
  bool button_active = g_state.input.selfright_switch;
  bool link_ok = g_state.input.link_ok;
  bool kill_active = g_state.input.kill_switch;

  if (button_active && link_ok && !kill_active) {
    // Button pressed - move to extended position
    target_us = SERVO_ENDPOINT_EXTEND;
  } else {
    // Button released, link lost, or kill switch active - return to neutral
    target_us = SERVO_NEUTRAL_US;
  }

  // Apply slew-rate limiting (slow movement to prevent brownouts)
  int16_t delta = (int16_t)target_us - (int16_t)g_servo_previous_us;

  if (delta > SERVO_SLEW_RATE_MAX) {
    // Moving toward extended position (slow ramp up)
    g_servo_previous_us += SERVO_SLEW_RATE_MAX;
  } else if (delta < -SERVO_SLEW_RATE_MAX) {
    // Moving toward neutral (slow ramp down)
    g_servo_previous_us -= SERVO_SLEW_RATE_MAX;
  } else {
    // Within slew rate limit - jump directly to target
    g_servo_previous_us = target_us;
  }

  // Clamp to safe endpoints (prevent mechanical binding)
  if (g_servo_previous_us < SERVO_ENDPOINT_RETRACT) {
    g_servo_previous_us = SERVO_ENDPOINT_RETRACT;
  } else if (g_servo_previous_us > SERVO_ENDPOINT_EXTEND) {
    g_servo_previous_us = SERVO_ENDPOINT_EXTEND;
  }

  return g_servo_previous_us;
}

// ============================================================================
// PUBLIC INTERFACE
// ============================================================================

void servo_init() {
  // Initialize servo output to neutral position (safe default)
  g_servo_previous_us = SERVO_NEUTRAL_US;
  g_state.output.servo_us = SERVO_NEUTRAL_US;
}

void servo_update() {
  // Calculate and apply servo output
  g_state.output.servo_us = servo_calculate_output();
}
