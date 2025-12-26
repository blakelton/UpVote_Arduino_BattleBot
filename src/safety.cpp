// safety.cpp - Safety module implementation
// UpVote Battlebot - Phase 1
#include "safety.h"
#include "config.h"
#include "state.h"

// ============================================================================
// SAFETY MODULE IMPLEMENTATION
// ============================================================================

void safety_init() {
  // Ensure system starts in safe state
  g_state.safety.arm_state = DISARMED;
  g_state.safety.error = ERR_NONE;

  // Phase 1.6 will add watchdog initialization here
}

bool safety_is_safe() {
  // Check all safety conditions
  // For Phase 1, just check that there are no errors
  // Phase 2+ will add link health check
  // Phase 5+ will add arming logic

  return (g_state.safety.error == ERR_NONE);
}

void safety_set_error(SystemError error) {
  // First-error-wins: don't overwrite existing errors
  if (g_state.safety.error == ERR_NONE) {
    g_state.safety.error = error;
  }
}
