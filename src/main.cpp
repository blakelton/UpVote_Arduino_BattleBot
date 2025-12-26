// main.cpp - UpVote Battlebot Main Control Loop
// Phase 1: Safety Scaffolding
#include <Arduino.h>
#include "config.h"
#include "state.h"
#include "safety.h"
#include "actuators.h"
#include "diagnostics.h"

// ============================================================================
// CONTROL LOOP TIMING
// ============================================================================

// Track loop timing for performance monitoring
static uint32_t next_loop_us = 0;

// ============================================================================
// SETUP - Runs once on boot
// ============================================================================
void setup() {
  // CRITICAL: Initialize outputs FIRST to ensure safe states
  // before any other code runs
  actuators_init();

  // Initialize safety system
  safety_init();

  // Initialize diagnostics (LED patterns)
  diagnostics_init();

  // Initialize loop timing
  next_loop_us = micros() + LOOP_PERIOD_US;
}

// ============================================================================
// LOOP - Runs repeatedly at 100 Hz (10ms period)
// ============================================================================
void loop() {
  // Wait for next loop iteration (maintains 100 Hz rate)
  uint32_t now_us = micros();
  if (now_us < next_loop_us) {
    // Still have time before next iteration
    // QA fix H3: Add power management here in future
    // For now, early return is acceptable for battlebot (always-on requirement)
    // TODO Phase 2+: Consider SLEEP_MODE_IDLE for power savings if needed
    return;
  }

  // Record loop start time for profiling
  g_state.loop_start_us = now_us;

  // Check for loop overrun (took longer than 10ms)
  // QA fix M5: Detection only - system continues with delayed timing
  // Recovery: Next loop will try to catch up (acceptable for Phase 1)
  // Future phases may add reduced functionality mode if needed
  if (now_us > next_loop_us + LOOP_PERIOD_US) {
    safety_set_error(ERR_LOOP_OVERRUN);
  }

  // Schedule next loop iteration
  next_loop_us += LOOP_PERIOD_US;

  // ========================================================================
  // CONTROL LOOP BODY (runs at 100 Hz)
  // ========================================================================

  // Reset hardware watchdog timer (must be called every loop)
  safety_watchdog_reset();

  // Phase 2: Input processing will go here
  // input_update();

  // Phase 4: Drive mixing will go here
  // mixer_update();

  // Phase 5: Weapon control will go here
  // weapon_update();

  // Phase 6: Servo control will go here
  // servo_update();

  // Update all actuator outputs
  actuators_update();

  // Phase 1.5: Update LED diagnostics
  diagnostics_update();

  // ========================================================================
  // END OF CONTROL LOOP
  // ========================================================================

  // Record loop execution time for profiling
  g_state.loop_duration_us = micros() - g_state.loop_start_us;
}
