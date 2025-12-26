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

  // Phase 1.5: Diagnostics initialization will go here
  // diagnostics_init();

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
    // Could add idle processing here if needed
    return;
  }

  // Record loop start time for profiling
  g_state.loop_start_us = now_us;

  // Check for loop overrun (took longer than 10ms)
  if (now_us > next_loop_us + LOOP_PERIOD_US) {
    safety_set_error(ERR_LOOP_OVERRUN);
  }

  // Schedule next loop iteration
  next_loop_us += LOOP_PERIOD_US;

  // ========================================================================
  // CONTROL LOOP BODY (runs at 100 Hz)
  // ========================================================================

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
  // diagnostics_update();

  // ========================================================================
  // END OF CONTROL LOOP
  // ========================================================================

  // Record loop execution time for profiling
  g_state.loop_duration_us = micros() - g_state.loop_start_us;
}
