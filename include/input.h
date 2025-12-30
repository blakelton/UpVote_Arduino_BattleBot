// input.h - CRSF receiver input module interface (AlfredoCRSF)
// UpVote Battlebot - Phase 7: AlfredoCRSF Integration
#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>
#include <AlfredoCRSF.h>

// ============================================================================
// ALFREDO CRSF LIBRARY INSTANCE
// ============================================================================

// Global CRSF instance (defined in input.cpp)
extern AlfredoCRSF crsf;

// ============================================================================
// INPUT CONFIGURATION
// ============================================================================

// Deadband for stick inputs (5% zone around center)
#define INPUT_DEADBAND  0.05f

// ============================================================================
// INPUT MODULE INTERFACE
// ============================================================================

// Initialize CRSF receiver input
// Sets up UART at 420kbaud for CRSF protocol
// Call this ONCE in setup() after safety_init()
void input_init();

// Update input state from CRSF receiver
// Processes incoming CRSF packets, updates RuntimeState.input
// Call this every control loop iteration (100 Hz)
void input_update();

// Send telemetry data back to TX16S transmitter
// Sends battery voltage, free RAM, and system status
// Call this every control loop iteration (100 Hz) - internally rate-limited to 1 Hz
void input_update_telemetry();

#endif // INPUT_H
