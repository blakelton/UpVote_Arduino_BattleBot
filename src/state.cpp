// state.cpp - Runtime state initialization
// UpVote Battlebot - Phase 1
#include "state.h"
#include "config.h"

// ============================================================================
// GLOBAL STATE VARIABLE
// ============================================================================

// Initialize all fields to safe defaults
RuntimeState g_state = {
  // Timing
  .loop_start_us = 0,
  .loop_duration_us = 0,

  // Input state
  .input = {
    .roll = 0.0f,
    .pitch = 0.0f,
    .yaw = 0.0f,
    .throttle = 0.0f,
    .weapon = 0.0f,
    .arm_switch = false,
    .kill_switch = false,
    .selfright_switch = false,
    .last_packet_ms = 0,
    .link_ok = false,
    .raw_channels = {992, 992, 992, 992, 992, 992, 992, 992, 992, 992, 992, 992, 992, 992, 992, 992}  // All at center
  },

  // Safety state
  .safety = {
    .arm_state = DISARMED,
    .error = ERR_NONE,
    .arm_switch_debounced = false,
    .kill_switch_debounced = false,
    .arm_switch_stable_ms = 0,
    .kill_switch_stable_ms = 0,
    .last_arm_throttle = 0.0f
  },

  // Output state (safe defaults)
  .output = {
    .motor_fl_pwm = SAFE_MOTOR_PWM,
    .motor_fr_pwm = SAFE_MOTOR_PWM,
    .motor_rl_pwm = SAFE_MOTOR_PWM,
    .motor_rr_pwm = SAFE_MOTOR_PWM,
    .weapon_us = SAFE_WEAPON_US,
    .servo_us = SAFE_SERVO_US
  },

  // Diagnostics state
  .diagnostics = {
    .led_last_update_ms = 0,
    .led_state = false,
    .error_blink_count = 0,
    .error_blink_phase = 0
  },

  // Battery telemetry state
  .battery = {
    .voltage = 0.0f,
    .percentage = 0,
    .last_telemetry_ms = 0
  }
};
