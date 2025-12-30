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
  // FULL 4-MOTOR SEQUENTIAL TEST
  // Tests each motor forward then backward in sequence
  // Frame-based timing (100Hz loop = 10ms per frame)

  static uint16_t frame_count = 0;
  frame_count++;

  // 8 phases x 2 seconds = 16 second total cycle
  uint8_t phase = (frame_count / 200) % 8;  // 200 frames = 2 seconds per phase

  const int16_t TEST_SPEED = 200;
  int16_t rl_cmd = 0, rr_cmd = 0, fl_cmd = 0, fr_cmd = 0;

  switch (phase) {
    case 0: rl_cmd = TEST_SPEED; break;     // Rear-Left forward
    case 1: rl_cmd = -TEST_SPEED; break;    // Rear-Left backward
    case 2: rr_cmd = TEST_SPEED; break;     // Rear-Right forward
    case 3: rr_cmd = -TEST_SPEED; break;    // Rear-Right backward
    case 4: fl_cmd = TEST_SPEED; break;     // Front-Left forward
    case 5: fl_cmd = -TEST_SPEED; break;    // Front-Left backward
    case 6: fr_cmd = TEST_SPEED; break;     // Front-Right forward
    case 7: fr_cmd = -TEST_SPEED; break;    // Front-Right backward
  }

  actuators_set_motor(0, rl_cmd);  // M1 (Rear-Left)
  actuators_set_motor(1, rr_cmd);  // M2 (Rear-Right)
  actuators_set_motor(2, fl_cmd);  // M4 (Front-Left)
  actuators_set_motor(3, fr_cmd);  // M3 (Front-Right)
}
