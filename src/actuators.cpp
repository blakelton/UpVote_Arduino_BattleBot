// actuators.cpp - Motor, ESC, and servo output implementation
// UpVote Battlebot - Phase 1
// Phase 7: AFMotor library integration for L293D V1 Motor Shield
#include "actuators.h"
#include "config.h"
#include "state.h"
#include <Arduino.h>
#include <AFMotor.h>  // Adafruit Motor Shield V1 library (required for L293D shield)

// ============================================================================
// AFMOTOR OBJECTS - L293D V1 Motor Shield Control
// ============================================================================

// Motor objects for L293D V1 Motor Shield
// PHASE 7 FIX: Use matching PWM frequencies for all motors
// Motor terminal mapping (verified via hardware documentation):
//   M1 terminal = Rear-Left (RL) motor
//   M2 terminal = Rear-Right (RR) motor
//   M3 terminal = Front-Right (FR) motor
//   M4 terminal = Front-Left (FL) motor
// PWM Frequencies: Use lowest available frequencies for consistent motor response
//   - MOTOR12_2KHZ = ~2 kHz (Timer2: for M1/M2)
//   - MOTOR34_1KHZ = ~1 kHz (Timer0: for M3/M4, lowest available on UNO)
static AF_DCMotor motor_rl(1, MOTOR12_2KHZ);  // M1: ~2 kHz
static AF_DCMotor motor_rr(2, MOTOR12_2KHZ);  // M2: ~2 kHz
static AF_DCMotor motor_fr(3, MOTOR34_1KHZ);  // M3: ~1 kHz (closer to M1/M2)
static AF_DCMotor motor_fl(4, MOTOR34_1KHZ);  // M4: ~1 kHz (closer to M1/M2)

// ============================================================================
// PRIVATE HELPER FUNCTIONS
// ============================================================================

// Shift register state (global to avoid ISR safety issues)
// Tracks current motor direction bits sent to 74HC595
// TODO Phase 7: Remove after AFMotor integration complete
static uint8_t g_shift_reg_state = 0x00;

// Write 8 bits to the 74HC595 shift register (motor directions)
static void shift_register_write(uint8_t data) {
  digitalWrite(PIN_SR_LATCH, LOW);  // Prepare to shift data
  shiftOut(PIN_SR_DATA, PIN_SR_CLOCK, MSBFIRST, data);
  digitalWrite(PIN_SR_LATCH, HIGH); // Latch data to outputs
}

// Set motor direction via shift register bits
// motor: 0=RL, 1=RR, 2=FL, 3=FR
// forward: true=forward, false=reverse
static void set_motor_direction(uint8_t motor, bool forward) {
  // Bounds check motor index (QA fix: M3)
  if (motor > 3) return;

  // Calculate bit positions for this motor
  uint8_t bit_a = motor * 2;     // Direction A bit
  uint8_t bit_b = motor * 2 + 1; // Direction B bit

  // Set direction bits
  if (forward) {
    g_shift_reg_state |= (1 << bit_a);   // A = HIGH
    g_shift_reg_state &= ~(1 << bit_b);  // B = LOW
  } else {
    g_shift_reg_state &= ~(1 << bit_a);  // A = LOW
    g_shift_reg_state |= (1 << bit_b);   // B = HIGH
  }

  shift_register_write(g_shift_reg_state);
}

// ============================================================================
// PHASE 3: MOTOR CONTROL WITH SLEW-RATE LIMITING
// ============================================================================

// Previous motor PWM values (for slew-rate limiting)
static int16_t g_motor_previous[4] = {0, 0, 0, 0};  // [RL, RR, FL, FR]

// Apply slew-rate limiting to a motor command
// current: Current motor value
// target: Target motor value
// Returns: Slewed value (gradually approaches target)
static int16_t apply_slew_rate(int16_t current, int16_t target) {
  int16_t delta = target - current;

  if (delta > MOTOR_SLEW_RATE_MAX) {
    return current + MOTOR_SLEW_RATE_MAX;
  } else if (delta < -MOTOR_SLEW_RATE_MAX) {
    return current - MOTOR_SLEW_RATE_MAX;
  } else {
    return target;
  }
}

// Motor polarity inversion lookup table
static const bool g_motor_inverted[4] = {
  MOTOR_RL_INVERTED,  // Motor 0: Rear-Left
  MOTOR_RR_INVERTED,  // Motor 1: Rear-Right
  MOTOR_FL_INVERTED,  // Motor 2: Front-Left
  MOTOR_FR_INVERTED   // Motor 3: Front-Right
};

// Initialize AFMotor objects to safe stopped state
static void init_afmotor_motors() {
  // Initialize all motors - set speed first, then release
  // This matches the pattern from working test code
  motor_rl.setSpeed(200);
  motor_rl.run(RELEASE);
  motor_rr.setSpeed(200);
  motor_rr.run(RELEASE);
  motor_fr.setSpeed(200);
  motor_fr.run(RELEASE);
  motor_fl.setSpeed(200);
  motor_fl.run(RELEASE);
}

// Front motor minimum PWM boost to overcome higher inertia/friction
// This is added to front motor speed when they're moving (non-zero command)
#define FRONT_MOTOR_BOOST 30  // Add 30 PWM counts to front motors

// Update motors using AFMotor library
static void update_afmotor_motors() {
  // Read motor commands from global state
  int16_t fl = g_state.output.motor_fl_pwm;
  int16_t fr = g_state.output.motor_fr_pwm;
  int16_t rl = g_state.output.motor_rl_pwm;
  int16_t rr = g_state.output.motor_rr_pwm;

  // Apply inversion flags BEFORE AFMotor control
  int16_t rl_adjusted = MOTOR_RL_INVERTED ? -rl : rl;
  int16_t rr_adjusted = MOTOR_RR_INVERTED ? -rr : rr;
  int16_t fr_adjusted = MOTOR_FR_INVERTED ? -fr : fr;
  int16_t fl_adjusted = MOTOR_FL_INVERTED ? -fl : fl;

  // Calculate base speeds
  uint8_t rl_speed = (uint8_t)constrain(abs(rl_adjusted), 0, 255);
  uint8_t rr_speed = (uint8_t)constrain(abs(rr_adjusted), 0, 255);
  uint8_t fr_speed = (uint8_t)constrain(abs(fr_adjusted), 0, 255);
  uint8_t fl_speed = (uint8_t)constrain(abs(fl_adjusted), 0, 255);

  // BOOST FRONT MOTORS: Add fixed offset when motor is commanded to move
  // This helps front motors overcome their higher inertia threshold
  if (fr_speed > 0) {
    fr_speed = (uint8_t)constrain(fr_speed + FRONT_MOTOR_BOOST, 0, 255);
  }
  if (fl_speed > 0) {
    fl_speed = (uint8_t)constrain(fl_speed + FRONT_MOTOR_BOOST, 0, 255);
  }

  // Update Rear-Left motor (M1 terminal)
  motor_rl.setSpeed(rl_speed);
  if (rl_speed == 0) {
    motor_rl.run(RELEASE);
  } else {
    motor_rl.run(rl_adjusted >= 0 ? FORWARD : BACKWARD);
  }

  // Update Rear-Right motor (M2 terminal)
  motor_rr.setSpeed(rr_speed);
  if (rr_speed == 0) {
    motor_rr.run(RELEASE);
  } else {
    motor_rr.run(rr_adjusted >= 0 ? FORWARD : BACKWARD);
  }

  // Update Front-Right motor (M3 terminal) - WITH BOOST APPLIED
  motor_fr.setSpeed(fr_speed);
  if (fr_speed == 0) {
    motor_fr.run(RELEASE);
  } else {
    motor_fr.run(fr_adjusted >= 0 ? FORWARD : BACKWARD);
  }

  // Update Front-Left motor (M4 terminal) - WITH BOOST APPLIED
  motor_fl.setSpeed(fl_speed);
  if (fl_speed == 0) {
    motor_fl.run(RELEASE);
  } else {
    motor_fl.run(fl_adjusted >= 0 ? FORWARD : BACKWARD);
  }
}

// ============================================================================
// PUBLIC INTERFACE
// ============================================================================

void actuators_set_motor(uint8_t motor_index, int16_t command) {
  // Bounds check motor index
  if (motor_index > 3) return;

  // PHASE 7 FIX: Removed polarity inversion from here
  // Inversion is now ONLY applied in update_afmotor_motors()
  // This prevents double-inversion bug that was breaking M2

  // Step 1: Apply global duty cycle clamp (thermal protection)
  int16_t adjusted_command = constrain(command, -MOTOR_DUTY_CLAMP_MAX, MOTOR_DUTY_CLAMP_MAX);

  // Step 2: SLEW RATE DISABLED FOR TESTING - Direct response
  // int16_t slewed_command = apply_slew_rate(g_motor_previous[motor_index], adjusted_command);
  // g_motor_previous[motor_index] = slewed_command;
  int16_t slewed_command = adjusted_command;  // Direct pass-through for testing

  // Step 3: Write to appropriate motor in g_state.output
  switch (motor_index) {
    case 0: g_state.output.motor_rl_pwm = slewed_command; break;  // Rear-Left
    case 1: g_state.output.motor_rr_pwm = slewed_command; break;  // Rear-Right
    case 2: g_state.output.motor_fl_pwm = slewed_command; break;  // Front-Left
    case 3: g_state.output.motor_fr_pwm = slewed_command; break;  // Front-Right
  }
}

void actuators_init() {
  // --- PHASE 7: AFMotor Library Initialization ---
  // Initialize motors using AFMotor library (handles shift register + PWM internally)
  // CRITICAL: Do NOT manually initialize shift register or motor PWM pins
  // AFMotor library handles all of this internally!
  init_afmotor_motors();

  // --- Phase 5/6: Initialize weapon ESC and servo pins ---
  // Note: These use Arduino's analogWrite() which generates PWM at ~490Hz
  // This is acceptable for battlebot use (ESC/servo will average the signal)
  // Production systems could use custom Timer2 PWM for proper 50Hz if needed
  pinMode(PIN_WEAPON_ESC, OUTPUT);
  pinMode(PIN_SELFRIGHT_SERVO, OUTPUT);

  // Write safe initial values (PWM duty cycle: 0-255 maps to 0-5V)
  // For ESC/servo: We'll map microseconds to duty cycle in actuators_update()
  analogWrite(PIN_WEAPON_ESC, 0);
  analogWrite(PIN_SELFRIGHT_SERVO, 0);

  // Note: CRSF pins (0, 1) are initialized by Serial in Phase 2
  // Note: Status LED pin is initialized by diagnostics module in Phase 1.5
}

void actuators_update() {
  // === PHASE 7: AFMotor Integration Switch ===
  // Set to 1 to use AFMotor library, 0 for old shift register code
  #define USE_AFMOTOR 1

  #if USE_AFMOTOR
    // NEW CODE: Update motors using AFMotor library
    update_afmotor_motors();
  #else
    // OLD CODE: Manual shift register + PWM control
    // Read output state from global state
    int16_t fl = g_state.output.motor_fl_pwm;
    int16_t fr = g_state.output.motor_fr_pwm;
    int16_t rl = g_state.output.motor_rl_pwm;
    int16_t rr = g_state.output.motor_rr_pwm;

    // --- Update Motor Directions ---
    // HARDWARE MAPPING FIX: Actual wiring is M3=FR, M4=FL (swapped from pin names)
    set_motor_direction(2, fr >= 0);  // M3 terminal = FR motor (hardware swapped)
    set_motor_direction(3, fl >= 0);  // M4 terminal = FL motor (hardware swapped)
    set_motor_direction(0, rl >= 0);  // M1 terminal = RL motor
    set_motor_direction(1, rr >= 0);  // M2 terminal = RR motor

    // --- Update Motor PWM (absolute value with bounds check) ---
    // HARDWARE MAPPING FIX: Write FR command to M3 terminal, FL command to M4 terminal
    analogWrite(PIN_MOTOR_FL_PWM, (uint8_t)constrain(abs(fr), MOTOR_PWM_MIN, MOTOR_PWM_MAX));  // D9=M3 has FR motor
    analogWrite(PIN_MOTOR_FR_PWM, (uint8_t)constrain(abs(fl), MOTOR_PWM_MIN, MOTOR_PWM_MAX));  // D10=M4 has FL motor
    analogWrite(PIN_MOTOR_RL_PWM, (uint8_t)constrain(abs(rl), MOTOR_PWM_MIN, MOTOR_PWM_MAX));  // D5=M1 has RL motor
    analogWrite(PIN_MOTOR_RR_PWM, (uint8_t)constrain(abs(rr), MOTOR_PWM_MIN, MOTOR_PWM_MAX));  // D6=M2 has RR motor
  #endif

  // --- Update Weapon ESC (Phase 5) ---
  // Map microseconds [1000-2000] to PWM duty cycle [0-255]
  // Note: Arduino analogWrite() generates ~490Hz PWM, not standard 50Hz servo PWM
  // ESCs will average the signal - works adequately for battlebot use
  uint16_t weapon_us = g_state.output.weapon_us;
  uint8_t weapon_pwm = map(constrain(weapon_us, WEAPON_ESC_MIN_US, WEAPON_ESC_MAX_US),
                            WEAPON_ESC_MIN_US, WEAPON_ESC_MAX_US, 0, 255);
  analogWrite(PIN_WEAPON_ESC, weapon_pwm);

  // --- Update Servo (Phase 6) ---
  // Map microseconds [700-2300] to PWM duty cycle [0-255]
  uint16_t servo_us = g_state.output.servo_us;
  uint8_t servo_pwm = map(constrain(servo_us, SERVO_ENDPOINT_RETRACT, SERVO_ENDPOINT_EXTEND),
                           SERVO_ENDPOINT_RETRACT, SERVO_ENDPOINT_EXTEND, 0, 255);
  analogWrite(PIN_SELFRIGHT_SERVO, servo_pwm);
}

void actuators_emergency_stop() {
  // Immediately stop all motors
  analogWrite(PIN_MOTOR_FL_PWM, SAFE_MOTOR_PWM);
  analogWrite(PIN_MOTOR_FR_PWM, SAFE_MOTOR_PWM);
  analogWrite(PIN_MOTOR_RL_PWM, SAFE_MOTOR_PWM);
  analogWrite(PIN_MOTOR_RR_PWM, SAFE_MOTOR_PWM);

  // Brake all motors (A=HIGH, B=HIGH)
  shift_register_write(0xFF);

  // Stop weapon
  analogWrite(PIN_WEAPON_ESC, 0);

  // Neutral servo
  analogWrite(PIN_SELFRIGHT_SERVO, 0);

  // Update state to reflect emergency stop
  g_state.output.motor_fl_pwm = SAFE_MOTOR_PWM;
  g_state.output.motor_fr_pwm = SAFE_MOTOR_PWM;
  g_state.output.motor_rl_pwm = SAFE_MOTOR_PWM;
  g_state.output.motor_rr_pwm = SAFE_MOTOR_PWM;
  g_state.output.weapon_us = SAFE_WEAPON_US;
  g_state.output.servo_us = SAFE_SERVO_US;
}
