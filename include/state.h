// state.h - Runtime state structures
// UpVote Battlebot - Phase 1
#ifndef STATE_H
#define STATE_H

#include <Arduino.h>

// ============================================================================
// ENUMS - System States and Modes
// ============================================================================

// Arming state (Phase 1.4+)
enum ArmState {
  DISARMED = 0,  // Weapon cannot activate
  ARMED = 1      // Weapon can activate
};

// System error codes (Phase 1.6+)
enum SystemError {
  ERR_NONE = 0,           // No error
  ERR_LOOP_OVERRUN = 1,   // Control loop took too long
  ERR_WATCHDOG_RESET = 2, // Recovered from watchdog reset
  ERR_CRSF_TIMEOUT = 3,   // CRSF link loss (Phase 2+)
  ERR_CRSF_CRC = 4        // CRSF CRC validation failed (Phase 2+)
};

// ============================================================================
// RUNTIME STATE STRUCTURE
// ============================================================================

// Central data structure holding all runtime state
// All modules read/write to this structure
struct RuntimeState {

  // --- Timing ---
  uint32_t loop_start_us;    // Loop start time (microseconds)
  uint32_t loop_duration_us; // Last loop execution time

  // --- Input State (Phase 2+) ---
  struct {
    float roll;       // Roll input [-1.0, +1.0] (right stick X)
    float pitch;      // Pitch input [-1.0, +1.0] (right stick Y)
    float yaw;        // Yaw input [-1.0, +1.0] (left stick X)
    float throttle;   // Throttle input [-1.0, +1.0] (left stick Y - unused for holonomic)
    float weapon;     // Weapon throttle [0.0, +1.0] (slider/pot)

    bool arm_switch;      // Arming switch state (SA/SB/SC)
    bool kill_switch;     // Kill switch state (SD)
    bool selfright_switch; // Self-right trigger (SE/SF)

    uint32_t last_packet_ms; // Time of last valid CRSF packet
    bool link_ok;            // Link health status
  } input;

  // --- Safety State (Phase 1.4+) ---
  struct {
    ArmState arm_state;      // Current arming state
    SystemError error;       // Current error code

    bool arm_switch_debounced;    // Debounced arm switch (Phase 5+)
    bool kill_switch_debounced;   // Debounced kill switch (Phase 5+)
    float last_arm_throttle;      // Throttle value when last armed (Phase 5+)
  } safety;

  // --- Output State (Phase 3+) ---
  struct {
    // Drive motors
    int16_t motor_fl_pwm;  // Front-left PWM [-255, +255]
    int16_t motor_fr_pwm;  // Front-right PWM [-255, +255]
    int16_t motor_rl_pwm;  // Rear-left PWM [-255, +255]
    int16_t motor_rr_pwm;  // Rear-right PWM [-255, +255]

    // Weapon ESC
    uint16_t weapon_us;    // Weapon ESC pulse width (microseconds)

    // Self-righting servo
    uint16_t servo_us;     // Servo pulse width (microseconds)
  } output;

  // --- Diagnostics State (Phase 1.5+) ---
  struct {
    uint32_t led_last_update_ms;  // Last LED update time
    bool led_state;               // Current LED on/off state
    uint8_t error_blink_count;    // Error code blink counter
    uint8_t error_blink_phase;    // Error code state machine phase
  } diagnostics;
};

// ============================================================================
// GLOBAL STATE VARIABLE (declared in state.cpp)
// ============================================================================

extern RuntimeState g_state;

#endif // STATE_H
