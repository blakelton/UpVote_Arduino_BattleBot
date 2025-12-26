// safety.h - Safety module interface
// UpVote Battlebot - Phase 1
#ifndef SAFETY_H
#define SAFETY_H

#include <Arduino.h>
#include "state.h"

// ============================================================================
// SAFETY MODULE INTERFACE
// ============================================================================

// Initialize safety system
// Sets safe defaults, prepares for operation
// Call this ONCE in setup() after actuators_init()
void safety_init();

// Check if system is in a safe state
// Returns true if all safety conditions are met
bool safety_is_safe();

// Get current arming state
inline ArmState safety_get_arm_state() {
  extern RuntimeState g_state;
  return g_state.safety.arm_state;
}

// Check if weapon is armed
inline bool safety_is_armed() {
  return safety_get_arm_state() == ARMED;
}

// Get current error code
inline SystemError safety_get_error() {
  extern RuntimeState g_state;
  return g_state.safety.error;
}

// Set error code (first-error-wins: doesn't overwrite existing errors)
void safety_set_error(SystemError error);

// Clear error code
inline void safety_clear_error() {
  extern RuntimeState g_state;
  g_state.safety.error = ERR_NONE;
}

// Reset watchdog timer (call this every control loop iteration)
// Must be called at least every 500ms to prevent system reset
void safety_watchdog_reset();

#endif // SAFETY_H
