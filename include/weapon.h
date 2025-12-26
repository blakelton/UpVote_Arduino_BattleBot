// weapon.h - Weapon control with arming state machine
// UpVote Battlebot - Phase 5
#ifndef WEAPON_H
#define WEAPON_H

#include <Arduino.h>

// ============================================================================
// WEAPON MODULE INTERFACE
// ============================================================================

// Initialize weapon control system
// Sets weapon to safe state (DISARMED, minimum throttle)
// Call this ONCE in setup() after safety_init()
void weapon_init();

// Update weapon control with arming state machine
// Handles:
// - Switch debouncing (ARM and KILL switches)
// - Arming preconditions (throttle near zero, link OK, etc.)
// - Disarming triggers (kill switch, link loss, etc.)
// - Weapon output scaling and ramping when armed
// Call this every control loop iteration (100 Hz)
void weapon_update();

#endif // WEAPON_H
