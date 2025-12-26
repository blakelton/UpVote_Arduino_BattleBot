// input.h - CRSF receiver input module interface
// UpVote Battlebot - Phase 2
#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>

// ============================================================================
// CRSF PROTOCOL CONSTANTS
// ============================================================================

// CRSF frame structure
#define CRSF_ADDRESS_FLIGHT_CONTROLLER  0xC8
#define CRSF_FRAMETYPE_RC_CHANNELS      0x16
#define CRSF_FRAMETYPE_BATTERY_SENSOR   0x08  // Telemetry: Battery sensor

// Frame size constraints
#define CRSF_FRAME_SIZE_MAX             64   // Maximum CRSF frame size
#define CRSF_PAYLOAD_SIZE_MAX           62   // Max payload (frame - addr - len)
#define CRSF_RC_CHANNELS_PAYLOAD_SIZE   22   // RC Channels payload size

// UART configuration
#define CRSF_BAUDRATE                   420000  // 420kbaud

// Channel constants
#define CRSF_CHANNEL_VALUE_MIN          172   // CRSF 11-bit minimum
#define CRSF_CHANNEL_VALUE_MID          992   // CRSF 11-bit center
#define CRSF_CHANNEL_VALUE_MAX          1811  // CRSF 11-bit maximum

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
