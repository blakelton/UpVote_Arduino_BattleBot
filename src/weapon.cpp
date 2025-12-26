// weapon.cpp - Weapon control with arming state machine
// UpVote Battlebot - Phase 5
#include "weapon.h"
#include "config.h"
#include "state.h"
#include "safety.h"
#include "utilities.h"

// ============================================================================
// PRIVATE STATE
// ============================================================================

// Previous weapon command (for slew-rate limiting)
static uint16_t g_weapon_previous_us = WEAPON_ESC_MIN_US;

// ============================================================================
// PRIVATE HELPER FUNCTIONS
// ============================================================================

// Debounce switch inputs
// Updates g_state.safety.arm_switch_debounced and kill_switch_debounced
static void weapon_update_switch_debounce() {
  uint32_t now = millis();

  // Debounce ARM switch using utility function
  debounce_switch(g_state.input.arm_switch,
                  &g_state.safety.arm_switch_debounced,
                  &g_state.safety.arm_switch_stable_ms,
                  SWITCH_DEBOUNCE_MS,
                  now);

  // Debounce KILL switch using utility function
  debounce_switch(g_state.input.kill_switch,
                  &g_state.safety.kill_switch_debounced,
                  &g_state.safety.kill_switch_stable_ms,
                  SWITCH_DEBOUNCE_MS,
                  now);
}

// Update arming state machine
// Handles preconditions, disarming triggers, and throttle hysteresis
static void weapon_update_arming() {
  bool arm_switch = g_state.safety.arm_switch_debounced;
  bool kill_active = g_state.safety.kill_switch_debounced;
  bool link_ok = g_state.input.link_ok;
  float throttle = g_state.input.weapon;
  SystemError error = g_state.safety.error;

  // ========================================================================
  // DISARMING CONDITIONS (highest priority - ANY condition disarms)
  // ========================================================================

  bool should_disarm = false;

  if (!arm_switch) should_disarm = true;      // ARM switch not active
  if (kill_active) should_disarm = true;      // Kill switch active
  if (!link_ok) should_disarm = true;         // Link loss
  if (error != ERR_NONE) should_disarm = true; // System error

  if (should_disarm) {
    g_state.safety.arm_state = DISARMED;
    // Store throttle value for hysteresis
    g_state.safety.last_arm_throttle = throttle;
    return; // Exit early
  }

  // ========================================================================
  // ARMING CONDITIONS (only if currently disarmed - ALL must be true)
  // ========================================================================

  if (g_state.safety.arm_state == DISARMED) {
    // Check throttle hysteresis
    // If we recently disarmed with high throttle, require throttle to drop
    // below REARM threshold before allowing arm
    bool throttle_ok = false;

    if (g_state.safety.last_arm_throttle > ARM_THROTTLE_THRESHOLD) {
      // Previously had high throttle, need to go lower
      if (throttle < REARM_THROTTLE_THRESHOLD) {
        throttle_ok = true;
        g_state.safety.last_arm_throttle = throttle; // Update stored value
      }
    } else {
      // Previously had low throttle, just check against ARM threshold
      if (throttle <= ARM_THROTTLE_THRESHOLD) {
        throttle_ok = true;
      }
    }

    // Check ALL arming preconditions
    bool can_arm = true;

    if (!arm_switch) can_arm = false;       // ARM switch must be active
    if (kill_active) can_arm = false;       // Kill switch must be inactive
    if (!link_ok) can_arm = false;          // Link must be healthy
    if (error != ERR_NONE) can_arm = false; // No system errors
    if (!throttle_ok) can_arm = false;      // Throttle must be near zero (with hysteresis)

    if (can_arm) {
      g_state.safety.arm_state = ARMED;
      g_state.safety.last_arm_throttle = throttle;
    }
  }

  // If already armed, state remains ARMED (unless disarm conditions triggered above)
}

// Calculate weapon output with slew-rate limiting
// Returns pulse width in microseconds [WEAPON_ESC_MIN_US, WEAPON_ESC_MAX_US]
static uint16_t weapon_calculate_output() {
  uint16_t target_us = WEAPON_ESC_MIN_US;

  if (g_state.safety.arm_state == ARMED) {
    // Scale weapon throttle [0.0, 1.0] to ESC range [MIN_US, MAX_US]
    float throttle = g_state.input.weapon;

    // Clamp throttle to valid range
    if (throttle < 0.0f) throttle = 0.0f;
    if (throttle > 1.0f) throttle = 1.0f;

    // Map to microseconds
    uint16_t range_us = WEAPON_ESC_MAX_US - WEAPON_ESC_MIN_US;
    target_us = WEAPON_ESC_MIN_US + (uint16_t)(throttle * range_us);
  } else {
    // Disarmed - always minimum throttle
    target_us = WEAPON_ESC_MIN_US;
  }

  // Apply slew-rate limiting (slower than drive motors for safety)
  int16_t delta = (int16_t)target_us - (int16_t)g_weapon_previous_us;

  if (delta > WEAPON_SLEW_RATE_MAX) {
    g_weapon_previous_us += WEAPON_SLEW_RATE_MAX;
  } else if (delta < -WEAPON_SLEW_RATE_MAX) {
    g_weapon_previous_us -= WEAPON_SLEW_RATE_MAX;
  } else {
    g_weapon_previous_us = target_us;
  }

  return g_weapon_previous_us;
}

// ============================================================================
// PUBLIC INTERFACE
// ============================================================================

void weapon_init() {
  // Initialize debounce state
  uint32_t now = millis();
  g_state.safety.arm_switch_stable_ms = now;
  g_state.safety.kill_switch_stable_ms = now;
  g_state.safety.arm_switch_debounced = false;
  g_state.safety.kill_switch_debounced = false;

  // Initialize arming state
  g_state.safety.arm_state = DISARMED;  // CRITICAL: Always boot to DISARMED
  g_state.safety.last_arm_throttle = 0.0f;

  // Initialize weapon output
  g_weapon_previous_us = WEAPON_ESC_MIN_US;
  g_state.output.weapon_us = WEAPON_ESC_MIN_US;
}

void weapon_update() {
  // Update switch debouncing
  weapon_update_switch_debounce();

  // Update arming state machine
  weapon_update_arming();

  // Calculate and apply weapon output
  g_state.output.weapon_us = weapon_calculate_output();
}
