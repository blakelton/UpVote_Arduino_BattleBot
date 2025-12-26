// config.h - Compile-time constants and configuration
// UpVote Battlebot - Phase 1
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// PIN DEFINITIONS - Based on Hardware Verification (Generic L293D Shield)
// ============================================================================
// See: docs/hardware/pin_assignments.md for full pinout details

// --- Drive Motors (via L293D Shield) ---
// Motor PWM pins (enable pins)
#define PIN_MOTOR_FL_PWM    9   // Front-Left motor (M3 on shield, Timer1)
#define PIN_MOTOR_FR_PWM   10   // Front-Right motor (M4 on shield, Timer1)
#define PIN_MOTOR_RL_PWM    5   // Rear-Left motor (M1 on shield, Timer0)
#define PIN_MOTOR_RR_PWM    6   // Rear-Right motor (M2 on shield, Timer0)

// 74HC595 Shift Register Control (for motor direction)
#define PIN_SR_LATCH       12   // Shift register latch (STcp)
#define PIN_SR_ENABLE       7   // Shift register output enable (OE, active LOW)
#define PIN_SR_DATA         8   // Serial data input (DS)
#define PIN_SR_CLOCK        4   // Shift clock (SHcp)

// --- Weapon ESC (bypasses shield, uses Timer2) ---
#define PIN_WEAPON_ESC      3   // Weapon motor ESC PWM signal (Timer2)

// --- Self-Righting Servo (bypasses shield, uses Timer2) ---
#define PIN_SELFRIGHT_SERVO 11  // Self-righting servo PWM signal (Timer2)

// --- CRSF Receiver (Hardware Serial) ---
#define PIN_CRSF_RX         0   // Hardware Serial RX (Arduino RX pin)
#define PIN_CRSF_TX         1   // Hardware Serial TX (Arduino TX pin)

// --- Diagnostics ---
#define PIN_STATUS_LED     LED_BUILTIN  // Status LED (pin 13)

// --- Optional: Battery Voltage Monitoring ---
// #define PIN_BATTERY_MONITOR A0  // Analog pin for voltage divider (optional)

// ============================================================================
// TIMING CONSTANTS
// ============================================================================

// Control loop timing
#define LOOP_RATE_HZ       100          // Target loop frequency (100 Hz = 10ms period)
#define LOOP_PERIOD_MS     (1000 / LOOP_RATE_HZ)  // 10ms
#define LOOP_PERIOD_US     (1000000UL / LOOP_RATE_HZ)  // 10000 microseconds

// ============================================================================
// MOTOR CONTROL CONSTANTS
// ============================================================================

// PWM values for motors (0-255)
#define MOTOR_PWM_MIN       0   // Motor stopped
#define MOTOR_PWM_MAX     255   // Motor full speed

// Motor direction bit positions in shift register
// Shift register bit layout (8 bits total):
// [M4_B | M4_A | M3_B | M3_A | M2_B | M2_A | M1_B | M1_A]
#define SR_M1_A  0  // Rear-Left motor, direction A
#define SR_M1_B  1  // Rear-Left motor, direction B
#define SR_M2_A  2  // Rear-Right motor, direction A
#define SR_M2_B  3  // Rear-Right motor, direction B
#define SR_M3_A  4  // Front-Left motor, direction A
#define SR_M3_B  5  // Front-Left motor, direction B
#define SR_M4_A  6  // Front-Right motor, direction A
#define SR_M4_B  7  // Front-Right motor, direction B

// Phase 3: Slew-rate limiting (Phase 3)
// Maximum PWM change per control loop iteration (100 Hz = 10ms)
// 25 units/tick = 0-255 in ~100ms (prevents current spikes)
#define MOTOR_SLEW_RATE_MAX  25

// Phase 3: Global duty cycle clamp (thermal protection)
// Limits maximum PWM duty cycle for all motors
// 255 = 100% duty, 204 = 80% duty (recommended for thermal safety)
#define MOTOR_DUTY_CLAMP_MAX  204  // 80% max duty cycle

// Phase 3: Motor polarity inversion flags
// Set to true to invert a motor's direction (corrects wiring polarity)
// Adjust these during hardware testing if motors spin backwards
#define MOTOR_FL_INVERTED  false
#define MOTOR_FR_INVERTED  false
#define MOTOR_RL_INVERTED  false
#define MOTOR_RR_INVERTED  false

// ============================================================================
// WEAPON ESC CONSTANTS
// ============================================================================

// Weapon ESC PWM timing (standard servo timing, 50 Hz)
#define WEAPON_ESC_FREQ_HZ    50    // 50 Hz PWM for ESC
#define WEAPON_ESC_MIN_US   1000    // Minimum pulse width (stopped)
#define WEAPON_ESC_MAX_US   2000    // Maximum pulse width (full throttle)
#define WEAPON_ESC_NEUTRAL_US 1500  // Neutral/idle (for calibration)

// ============================================================================
// SERVO CONSTANTS
// ============================================================================

// Self-righting servo timing (standard servo, 50 Hz)
#define SERVO_FREQ_HZ        50     // 50 Hz PWM
#define SERVO_MIN_US        544     // Minimum pulse width (0 degrees)
#define SERVO_MAX_US       2400     // Maximum pulse width (180 degrees)
#define SERVO_NEUTRAL_US   1500     // Neutral position (90 degrees)

// ============================================================================
// SAFETY CONSTANTS
// ============================================================================

// Failsafe timeouts
#define LINK_TIMEOUT_MS     200     // Link loss timeout (200ms without valid packet)
#define WATCHDOG_TIMEOUT_S    1     // Hardware watchdog timeout (1 second)

// Safe default values
#define SAFE_MOTOR_PWM        0     // Motors stopped
#define SAFE_WEAPON_US     WEAPON_ESC_MIN_US  // Weapon at minimum throttle
#define SAFE_SERVO_US      SERVO_NEUTRAL_US   // Servo at neutral position

// ============================================================================
// LED DIAGNOSTIC TIMING
// ============================================================================

// LED blink rates (in milliseconds)
#define LED_BLINK_SAFE_MS     500   // SAFE state: 1 Hz (500ms on, 500ms off)
#define LED_BLINK_ARMED_MS    100   // ARMED state: 5 Hz (100ms on, 100ms off)
#define LED_ERROR_BLINK_MS    100   // Error code blink duration (100ms on/off)
#define LED_ERROR_PAUSE_MS   1000   // Pause between error code sequences

// ============================================================================
// INPUT PROCESSING CONSTANTS
// ============================================================================

// Deadband threshold (prevents stick drift at center)
#define INPUT_DEADBAND  0.05f  // 5% deadband zone around center

// ============================================================================
// CRSF TELEMETRY CONSTANTS
// ============================================================================

// Telemetry configuration
#define CRSF_TELEMETRY_ENABLED  1     // Set to 0 to disable telemetry
#define TELEMETRY_UPDATE_MS     1000  // Send telemetry every 1 second (1 Hz)

// Battery voltage monitoring (optional)
// Uncomment and adjust if you add a voltage divider to A0
// #define BATTERY_MONITOR_PIN     A0
// #define BATTERY_SCALE_FACTOR    0.0049  // Calibration factor (ADC to volts)

// ============================================================================
// HOLONOMIC MIXING CONSTANTS (Phase 4)
// ============================================================================

// Drive mode parameters
// Beginner mode: 50% max duty, gentle control
#define BEGINNER_MAX_DUTY      127  // 50% of 255
#define BEGINNER_EXPO          0.3f // Light exponential curve

// Normal mode: 80% max duty (same as thermal clamp), balanced control
#define NORMAL_MAX_DUTY        204  // 80% of 255 (same as MOTOR_DUTY_CLAMP_MAX)
#define NORMAL_EXPO            0.2f // Moderate exponential curve

// Aggressive mode: 100% max duty (still respects thermal clamp), responsive control
#define AGGRESSIVE_MAX_DUTY    255  // 100% of 255
#define AGGRESSIVE_EXPO        0.1f // Minimal exponential curve

// Axis scaling (reduce rotation sensitivity relative to translation)
#define ROTATION_SCALE         0.7f  // 70% rotation sensitivity

// ============================================================================
// MEMORY BUDGET TRACKING
// ============================================================================

// Phase-by-phase memory budgets (bytes)
#define PHASE1_RAM_BUDGET_BYTES  512   // Phase 1: 25% of 2KB
#define PHASE2_RAM_BUDGET_BYTES  768   // Phase 2: 37.5% of 2KB
#define PHASE3_RAM_BUDGET_BYTES 1024   // Phase 3: 50% of 2KB
#define PHASE4_RAM_BUDGET_BYTES 1280   // Phase 4: 62.5% of 2KB
#define PHASE5_RAM_BUDGET_BYTES 1536   // Phase 5-6: 75% of 2KB
#define PHASE7_RAM_BUDGET_BYTES 1800   // Phase 7: 87.5% of 2KB (10% margin)

#endif // CONFIG_H
