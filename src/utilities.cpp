// utilities.cpp - Common utility functions implementation
// UpVote Battlebot - Phase 7
#include "utilities.h"

// ============================================================================
// SWITCH DEBOUNCING UTILITY
// ============================================================================

void debounce_switch(bool raw_state,
                     bool *debounced_state,
                     uint32_t *stable_time_ms,
                     uint8_t debounce_ms,
                     uint32_t now) {
  // Check if raw state matches debounced state
  if (raw_state != *debounced_state) {
    // State changed, check if it's been stable long enough
    if ((now - *stable_time_ms) >= debounce_ms) {
      // Stable for long enough, accept the change
      *debounced_state = raw_state;
    }
  } else {
    // State matches, reset the stability timer
    *stable_time_ms = now;
  }
}
