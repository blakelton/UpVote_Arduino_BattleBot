// diagnostics.cpp - LED diagnostics and system monitoring implementation
// UpVote Battlebot - Phase 1
#include "diagnostics.h"
#include "config.h"
#include "state.h"
#include "safety.h"

// ============================================================================
// PRIVATE HELPER FUNCTIONS
// ============================================================================

// Blink error code: N blinks, pause, repeat
static void diagnostics_blink_error_code(SystemError error) {
  static SystemError last_error = ERR_NONE;  // Track error changes (GAP-M4 fix)
  uint32_t now = millis();
  uint8_t blink_count = (uint8_t)error;  // Error enum value = blink count

  // Reset state if error changed mid-sequence (GAP-M4 fix)
  // Prevents confusing mixed patterns when error code changes
  if (error != last_error) {
    g_state.diagnostics.error_blink_phase = 0;
    g_state.diagnostics.error_blink_count = 0;
    g_state.diagnostics.led_last_update_ms = now;
    g_state.diagnostics.led_state = false;
    digitalWrite(PIN_STATUS_LED, LOW);
    last_error = error;
  }

  // State machine for error code blinking
  // Phase 0: Blink N times (100ms on, 100ms off)
  // Phase 1: Long pause (1000ms)
  // Repeat

  if (g_state.diagnostics.error_blink_phase == 0) {
    // Blinking phase
    if (now - g_state.diagnostics.led_last_update_ms >= LED_ERROR_BLINK_MS) {
      g_state.diagnostics.led_last_update_ms = now;

      if (g_state.diagnostics.led_state) {
        // Was ON, turn OFF
        digitalWrite(PIN_STATUS_LED, LOW);
        g_state.diagnostics.led_state = false;
        g_state.diagnostics.error_blink_count++;

        if (g_state.diagnostics.error_blink_count >= blink_count) {
          // Done blinking, enter pause
          g_state.diagnostics.error_blink_phase = 1;
          g_state.diagnostics.error_blink_count = 0;
        }
      } else {
        // Was OFF, turn ON
        digitalWrite(PIN_STATUS_LED, HIGH);
        g_state.diagnostics.led_state = true;
      }
    }
  } else {
    // Pause phase
    digitalWrite(PIN_STATUS_LED, LOW);
    if (now - g_state.diagnostics.led_last_update_ms >= LED_ERROR_PAUSE_MS) {
      // Pause done, restart blinking
      g_state.diagnostics.error_blink_phase = 0;
      g_state.diagnostics.led_last_update_ms = now;
    }
  }
}

// ============================================================================
// PUBLIC INTERFACE
// ============================================================================

void diagnostics_init() {
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);

  // Initialize diagnostics state
  g_state.diagnostics.led_last_update_ms = 0;
  g_state.diagnostics.led_state = false;
  g_state.diagnostics.error_blink_count = 0;
  g_state.diagnostics.error_blink_phase = 0;
}

void diagnostics_update() {
  uint32_t now = millis();

  // Determine current system status
  SystemError error = safety_get_error();
  bool link_ok = g_state.input.link_ok;  // Phase 2+ will set this
  bool armed = safety_is_armed();

  SystemStatus status;
  if (error != ERR_NONE) {
    status = STATUS_ERROR;
  } else if (!link_ok) {
    status = STATUS_FAILSAFE;
  } else if (armed) {
    status = STATUS_ARMED;
  } else {
    status = STATUS_SAFE;
  }

  // Execute LED pattern based on status
  switch (status) {
    case STATUS_SAFE:
      // Slow blink: 1 Hz (500ms on, 500ms off)
      if (now - g_state.diagnostics.led_last_update_ms >= LED_BLINK_SAFE_MS) {
        g_state.diagnostics.led_last_update_ms = now;
        g_state.diagnostics.led_state = !g_state.diagnostics.led_state;
        digitalWrite(PIN_STATUS_LED, g_state.diagnostics.led_state);
      }
      break;

    case STATUS_ARMED:
      // Fast blink: 5 Hz (100ms on, 100ms off)
      if (now - g_state.diagnostics.led_last_update_ms >= LED_BLINK_ARMED_MS) {
        g_state.diagnostics.led_last_update_ms = now;
        g_state.diagnostics.led_state = !g_state.diagnostics.led_state;
        digitalWrite(PIN_STATUS_LED, g_state.diagnostics.led_state);
      }
      break;

    case STATUS_FAILSAFE:
      // Solid ON
      digitalWrite(PIN_STATUS_LED, HIGH);
      break;

    case STATUS_ERROR:
      // Blink error code: N blinks, pause, repeat
      diagnostics_blink_error_code(error);
      break;
  }
}

int diagnostics_get_free_ram() {
  // AVR-specific: Calculate free RAM between stack and heap
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
