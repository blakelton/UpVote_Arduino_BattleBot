// diagnostics.h - LED diagnostics and system monitoring
// UpVote Battlebot - Phase 1
#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include <Arduino.h>

// ============================================================================
// SYSTEM STATUS ENUM
// ============================================================================

enum SystemStatus {
  STATUS_SAFE,      // Slow blink (1 Hz) - System OK, disarmed
  STATUS_ARMED,     // Fast blink (5 Hz) - Weapon armed
  STATUS_FAILSAFE,  // Solid ON - Link loss or kill active
  STATUS_ERROR      // Error code blink sequence
};

// ============================================================================
// DIAGNOSTICS MODULE INTERFACE
// ============================================================================

// Initialize diagnostics system (LED pin, etc.)
void diagnostics_init();

// Update LED state based on current system status
// Call this every control loop iteration
void diagnostics_update();

// Get free RAM (for CRSF telemetry in Phase 2+)
int diagnostics_get_free_ram();

#endif // DIAGNOSTICS_H
