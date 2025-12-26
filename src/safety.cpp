// safety.cpp - Safety module implementation
// UpVote Battlebot - Phase 1
#include "safety.h"
#include "config.h"
#include "state.h"
#include <avr/wdt.h>

// ============================================================================
// SAFETY MODULE IMPLEMENTATION
// ============================================================================

void safety_init() {
  // Check if reset was caused by watchdog
  // MCUSR (MCU Status Register) preserves reset cause
  uint8_t mcusr = MCUSR;
  MCUSR = 0;  // Clear reset flags

  // Ensure system starts in safe state
  g_state.safety.arm_state = DISARMED;
  g_state.safety.error = ERR_NONE;

  // If watchdog caused the reset, record it
  if (mcusr & (1 << WDRF)) {
    safety_set_error(ERR_WATCHDOG_RESET);
  }

  // Enable hardware watchdog timer
  // WDTO_500MS = 500ms timeout (loop runs at 10ms, so 50x safety margin)
  // If we miss 50 consecutive loop iterations, watchdog will reset the system
  wdt_enable(WDTO_500MS);
}

bool safety_is_safe() {
  // Check all safety conditions
  // For Phase 1, just check that there are no errors
  // Phase 2+ will add link health check
  // Phase 5+ will add arming logic

  // Validate state integrity (QA fix: H2)
  if (g_state.safety.error > ERR_CRSF_CRC) {
    // Invalid error code detected
    return false;
  }

  return (g_state.safety.error == ERR_NONE);
}

void safety_set_error(SystemError error) {
  // First-error-wins: don't overwrite existing errors
  if (g_state.safety.error == ERR_NONE) {
    g_state.safety.error = error;
  }
}

void safety_watchdog_reset() {
  // Reset hardware watchdog timer
  // Must be called at least every 500ms (we call it every 10ms)
  wdt_reset();
}
