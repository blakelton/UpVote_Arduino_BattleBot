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
  // CRSF 11-bit: 172 (min) - 991/992 (center) - 1811 (max)
  // Deadband: ~5% around center = ~41 counts each side (950-1033)
  const int16_t CENTER = 992;
  const int16_t DEADBAND = 41;  // ~5% of full range

  // Get max duty from current drive mode
  int16_t max_duty = g_mode_params.max_duty;
  float expo = g_mode_params.expo;

  // Read raw channels
  int16_t ch0_raw = g_state.input.raw_channels[0];  // Roll
  int16_t ch1_raw = g_state.input.raw_channels[1];  // Pitch
  int16_t ch3_raw = g_state.input.raw_channels[3];  // Yaw

  // Apply deadband and get normalized input [-1.0, +1.0]
  float x_norm, y_norm, r_norm;

  // Roll (X axis) - inverted for correct strafe direction
  if (abs(ch0_raw - CENTER) < DEADBAND) {
    x_norm = 0.0f;
  } else if (ch0_raw > CENTER) {
    x_norm = -(float)(ch0_raw - CENTER - DEADBAND) / (float)(1811 - CENTER - DEADBAND);
  } else {
    x_norm = -(float)(ch0_raw - CENTER + DEADBAND) / (float)(CENTER - DEADBAND - 172);
  }

  // Pitch (Y axis)
  if (abs(ch1_raw - CENTER) < DEADBAND) {
    y_norm = 0.0f;
  } else if (ch1_raw > CENTER) {
    y_norm = (float)(ch1_raw - CENTER - DEADBAND) / (float)(1811 - CENTER - DEADBAND);
  } else {
    y_norm = (float)(ch1_raw - CENTER + DEADBAND) / (float)(CENTER - DEADBAND - 172);
  }

  // Yaw (R axis)
  if (abs(ch3_raw - CENTER) < DEADBAND) {
    r_norm = 0.0f;
  } else if (ch3_raw > CENTER) {
    r_norm = (float)(ch3_raw - CENTER - DEADBAND) / (float)(1811 - CENTER - DEADBAND);
  } else {
    r_norm = (float)(ch3_raw - CENTER + DEADBAND) / (float)(CENTER - DEADBAND - 172);
  }

  // Apply exponential curve for smoother control
  x_norm = apply_expo(x_norm, expo);
  y_norm = apply_expo(y_norm, expo);
  r_norm = apply_expo(r_norm, expo);

  // Holonomic mixing with normalized values
  float fl_norm = y_norm + x_norm + r_norm;  // Front-Left
  float fr_norm = y_norm - x_norm - r_norm;  // Front-Right
  float rl_norm = y_norm - x_norm + r_norm;  // Rear-Left
  float rr_norm = y_norm + x_norm - r_norm;  // Rear-Right

  // Normalize to prevent saturation (keep ratios, limit max to 1.0)
  normalize_outputs(&fl_norm, &fr_norm, &rl_norm, &rr_norm);

  // Scale to PWM range based on drive mode max_duty
  int16_t fl_pwm = (int16_t)(fl_norm * max_duty);
  int16_t fr_pwm = (int16_t)(fr_norm * max_duty);
  int16_t rl_pwm = (int16_t)(rl_norm * max_duty);
  int16_t rr_pwm = (int16_t)(rr_norm * max_duty);

  // Send to motors
  actuators_set_motor(0, rl_pwm);  // M1: Rear-Left
  actuators_set_motor(1, rr_pwm);  // M2: Rear-Right
  actuators_set_motor(2, fl_pwm);  // M4: Front-Left
  actuators_set_motor(3, fr_pwm);  // M3: Front-Right
}
