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

// ============================================================================
// PHASE 3: INDIVIDUAL MOTOR CONTROL
// ============================================================================

// Set individual motor command with slew-rate limiting and polarity correction
// motor_index: 0=RL, 1=RR, 2=FL, 3=FR
// command: Signed command [-255, +255] (positive = forward, negative = reverse)
// This function applies:
// - Polarity inversion (if motor is flagged as inverted)
// - Global duty cycle clamping (thermal protection)
// - Slew-rate limiting (prevents current spikes)
// Updates g_state.output with the clamped/slewed value
void actuators_set_motor(uint8_t motor_index, int16_t command);

#endif // ACTUATORS_H
