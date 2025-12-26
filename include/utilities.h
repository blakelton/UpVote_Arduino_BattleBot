// utilities.h - Common utility functions
// UpVote Battlebot - Phase 7
#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// SWITCH DEBOUNCING UTILITY
// ============================================================================

/**
 * @brief Debounce a binary switch input
 *
 * This utility function implements time-based switch debouncing to filter
 * mechanical switch bounce. The switch must remain in a new state for the
 * specified debounce time before the debounced output changes.
 *
 * @param raw_state Current raw switch state (true/false)
 * @param debounced_state Pointer to debounced state variable (in/out)
 * @param stable_time_ms Pointer to stability timer variable (in/out)
 * @param debounce_ms Debounce time in milliseconds (typically 10ms)
 * @param now Current time in milliseconds (from millis())
 *
 * @note This function modifies *debounced_state and *stable_time_ms
 *
 * @example
 *   static bool button_debounced = false;
 *   static uint32_t button_stable_ms = 0;
 *
 *   void update() {
 *     bool button_raw = digitalRead(PIN_BUTTON);
 *     debounce_switch(button_raw, &button_debounced, &button_stable_ms, 10, millis());
 *     if (button_debounced) {
 *       // Button is pressed (debounced)
 *     }
 *   }
 */
void debounce_switch(bool raw_state,
                     bool *debounced_state,
                     uint32_t *stable_time_ms,
                     uint8_t debounce_ms,
                     uint32_t now);

#endif  // UTILITIES_H
