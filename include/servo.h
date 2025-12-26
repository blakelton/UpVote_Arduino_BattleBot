// servo.h - Self-righting servo control
// UpVote Battlebot - Phase 6
#ifndef SERVO_H
#define SERVO_H

#include <Arduino.h>

// ============================================================================
// SERVO MODULE INTERFACE
// ============================================================================

// Initialize servo control system
// Sets servo to safe state (neutral position)
// Call this ONCE in setup() after actuators_init()
void servo_init();

// Update servo control with momentary button logic
// Handles:
// - Momentary button control (CH7: selfright_switch)
// - Rate limiting to prevent brownouts
// - Endpoint clamping to safe calibrated range
// - Failsafe behavior (returns to neutral on link loss)
// Call this every control loop iteration (100 Hz)
void servo_update();

#endif // SERVO_H
