// mixing.h - Holonomic drive mixing module interface
// UpVote Battlebot - Phase 4
#ifndef MIXING_H
#define MIXING_H

#include <Arduino.h>

// ============================================================================
// DRIVE MODE ENUMERATION
// ============================================================================

// Drive modes for different skill levels / situations
enum DriveMode {
  DRIVE_MODE_BEGINNER = 0,    // 50% max speed, gentle expo
  DRIVE_MODE_NORMAL = 1,      // 80% max speed, moderate expo
  DRIVE_MODE_AGGRESSIVE = 2   // 100% max speed, minimal expo
};

// ============================================================================
// MIXING MODULE INTERFACE
// ============================================================================

// Initialize mixing module
// Sets default drive mode to NORMAL
// Call this ONCE in setup() after safety_init()
void mixing_init();

// Set active drive mode
// mode: Drive mode (BEGINNER, NORMAL, or AGGRESSIVE)
// Updates max duty cycle and expo curve parameters
void mixing_set_drive_mode(DriveMode mode);

// Get current drive mode
// Returns: Active drive mode
DriveMode mixing_get_drive_mode();

// Perform holonomic mixing
// Reads g_state.input (roll, pitch, yaw)
// Applies expo curves, holonomic math, and normalization
// Outputs via actuators_set_motor() for all 4 motors
// Call this every control loop iteration (100 Hz) when not in failsafe
void mixing_update();

#endif // MIXING_H
