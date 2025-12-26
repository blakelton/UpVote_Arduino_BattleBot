// actuators.h - Motor, ESC, and servo output interface
// UpVote Battlebot - Phase 1
#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <Arduino.h>

// ============================================================================
// ACTUATORS MODULE INTERFACE
// ============================================================================

// Initialize all actuator outputs to safe states
// Call this ONCE in setup() before anything else
void actuators_init();

// Write current output state to all actuators
// Call this every control loop iteration
// Reads from g_state.output and writes to hardware
void actuators_update();

// Emergency stop - force all outputs to safe states immediately
// Bypasses g_state and directly commands hardware
void actuators_emergency_stop();

#endif // ACTUATORS_H
