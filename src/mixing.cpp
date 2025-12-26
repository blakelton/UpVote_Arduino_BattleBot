// mixing.cpp - Holonomic drive mixing implementation
// UpVote Battlebot - Phase 4
#include "mixing.h"
#include "config.h"
#include "state.h"
#include "actuators.h"

// ============================================================================
// PRIVATE STATE
// ============================================================================

// Current drive mode
static DriveMode g_drive_mode = DRIVE_MODE_NORMAL;

// Current mode parameters (cached for performance)
static struct {
  uint8_t max_duty;  // Maximum PWM duty cycle for this mode
  float expo;        // Exponential curve factor
} g_mode_params;

// ============================================================================
// PRIVATE HELPER FUNCTIONS
// ============================================================================

// Apply exponential curve to input
// input: Normalized input [-1.0, +1.0]
// expo: Expo factor [0.0, 1.0] (0 = linear, 1 = full exponential)
// Returns: Expo-curved output [-1.0, +1.0]
static float apply_expo(float input, float expo) {
  // Expo formula: output = expo*input^3 + (1-expo)*input
  // This preserves sign and creates a smooth curve
  float input_cubed = input * input * input;
  return expo * input_cubed + (1.0f - expo) * input;
}

// Normalize motor outputs to prevent saturation
// Divides all outputs by the maximum absolute value if > 1.0
// This preserves the ratio of motor speeds while keeping all in [-1.0, +1.0]
static void normalize_outputs(float* fl, float* fr, float* rl, float* rr) {
  // Find maximum absolute value
  float max_abs = 1.0f;  // Start at 1.0 (no scaling needed if all outputs are within range)

  if (fabs(*fl) > max_abs) max_abs = fabs(*fl);
  if (fabs(*fr) > max_abs) max_abs = fabs(*fr);
  if (fabs(*rl) > max_abs) max_abs = fabs(*rl);
  if (fabs(*rr) > max_abs) max_abs = fabs(*rr);

  // Normalize by dividing all by max_abs
  if (max_abs > 1.0f) {
    *fl /= max_abs;
    *fr /= max_abs;
    *rl /= max_abs;
    *rr /= max_abs;
  }
}

// Update mode parameters cache from current drive mode
static void update_mode_params() {
  switch (g_drive_mode) {
    case DRIVE_MODE_BEGINNER:
      g_mode_params.max_duty = BEGINNER_MAX_DUTY;
      g_mode_params.expo = BEGINNER_EXPO;
      break;

    case DRIVE_MODE_NORMAL:
      g_mode_params.max_duty = NORMAL_MAX_DUTY;
      g_mode_params.expo = NORMAL_EXPO;
      break;

    case DRIVE_MODE_AGGRESSIVE:
      g_mode_params.max_duty = AGGRESSIVE_MAX_DUTY;
      g_mode_params.expo = AGGRESSIVE_EXPO;
      break;

    default:
      // Fallback to NORMAL if invalid mode
      g_mode_params.max_duty = NORMAL_MAX_DUTY;
      g_mode_params.expo = NORMAL_EXPO;
      break;
  }
}

// ============================================================================
// PUBLIC INTERFACE
// ============================================================================

void mixing_init() {
  // Set default drive mode
  g_drive_mode = DRIVE_MODE_NORMAL;
  update_mode_params();
}

void mixing_set_drive_mode(DriveMode mode) {
  // Validate mode
  if (mode > DRIVE_MODE_AGGRESSIVE) {
    mode = DRIVE_MODE_NORMAL;  // Fallback to safe default
  }

  g_drive_mode = mode;
  update_mode_params();
}

DriveMode mixing_get_drive_mode() {
  return g_drive_mode;
}

void mixing_update() {
  // Read inputs from global state (already normalized and deadbanded)
  float x = g_state.input.roll;   // Strafe (right stick X)
  float y = g_state.input.pitch;  // Forward/back (right stick Y)
  float r = g_state.input.yaw;    // Rotation (left stick X)

  // Apply exponential curves for better feel
  x = apply_expo(x, g_mode_params.expo);
  y = apply_expo(y, g_mode_params.expo);
  r = apply_expo(r, g_mode_params.expo);

  // Apply rotation scaling (reduce rotation sensitivity)
  r *= ROTATION_SCALE;

  // Holonomic mixing (mecanum/omni-wheel formula)
  // FL = Y + X + R
  // FR = Y - X - R
  // RL = Y - X + R
  // RR = Y + X - R
  float fl_raw = y + x + r;
  float fr_raw = y - x - r;
  float rl_raw = y - x + r;
  float rr_raw = y + x - r;

  // Normalize to prevent saturation artifacts
  normalize_outputs(&fl_raw, &fr_raw, &rl_raw, &rr_raw);

  // Apply mode-specific max duty cycle
  // Scale from [-1.0, +1.0] to [-max_duty, +max_duty]
  int16_t fl_pwm = (int16_t)(fl_raw * g_mode_params.max_duty);
  int16_t fr_pwm = (int16_t)(fr_raw * g_mode_params.max_duty);
  int16_t rl_pwm = (int16_t)(rl_raw * g_mode_params.max_duty);
  int16_t rr_pwm = (int16_t)(rr_raw * g_mode_params.max_duty);

  // Send commands to actuators module
  // actuators_set_motor() will apply:
  // - Polarity inversion (if configured)
  // - Global duty clamp (thermal protection)
  // - Slew-rate limiting (prevents brownouts)
  actuators_set_motor(2, fl_pwm);  // Motor 2: Front-Left
  actuators_set_motor(3, fr_pwm);  // Motor 3: Front-Right
  actuators_set_motor(0, rl_pwm);  // Motor 0: Rear-Left
  actuators_set_motor(1, rr_pwm);  // Motor 1: Rear-Right
}
