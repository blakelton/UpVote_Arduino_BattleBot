// actuators.cpp - Motor, ESC, and servo output implementation
// UpVote Battlebot - Phase 1
#include "actuators.h"
#include "config.h"
#include "state.h"
#include <Arduino.h>

// ============================================================================
// PRIVATE HELPER FUNCTIONS
// ============================================================================

// Shift register state (global to avoid ISR safety issues)
// Tracks current motor direction bits sent to 74HC595
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

// ============================================================================
// PUBLIC INTERFACE
// ============================================================================

void actuators_set_motor(uint8_t motor_index, int16_t command) {
  // Bounds check motor index
  if (motor_index > 3) return;

  // Step 1: Apply polarity inversion if configured
  int16_t adjusted_command = command;
  if (g_motor_inverted[motor_index]) {
    adjusted_command = -command;
  }

  // Step 2: Apply global duty cycle clamp (thermal protection)
  adjusted_command = constrain(adjusted_command, -MOTOR_DUTY_CLAMP_MAX, MOTOR_DUTY_CLAMP_MAX);

  // Step 3: Apply slew-rate limiting (prevents current spikes)
  int16_t slewed_command = apply_slew_rate(g_motor_previous[motor_index], adjusted_command);
  g_motor_previous[motor_index] = slewed_command;

  // Step 4: Write to appropriate motor in g_state.output
  switch (motor_index) {
    case 0: g_state.output.motor_rl_pwm = slewed_command; break;  // Rear-Left
    case 1: g_state.output.motor_rr_pwm = slewed_command; break;  // Rear-Right
    case 2: g_state.output.motor_fl_pwm = slewed_command; break;  // Front-Left
    case 3: g_state.output.motor_fr_pwm = slewed_command; break;  // Front-Right
  }
}

void actuators_init() {
  // --- Shift Register Pins ---
  pinMode(PIN_SR_LATCH, OUTPUT);
  pinMode(PIN_SR_ENABLE, OUTPUT);
  pinMode(PIN_SR_DATA, OUTPUT);
  pinMode(PIN_SR_CLOCK, OUTPUT);

  digitalWrite(PIN_SR_ENABLE, LOW);  // Enable shift register outputs
  shift_register_write(0x00);        // All motor directions to brake (A=0, B=0)

  // --- Motor PWM Pins ---
  pinMode(PIN_MOTOR_FL_PWM, OUTPUT);
  pinMode(PIN_MOTOR_FR_PWM, OUTPUT);
  pinMode(PIN_MOTOR_RL_PWM, OUTPUT);
  pinMode(PIN_MOTOR_RR_PWM, OUTPUT);

  analogWrite(PIN_MOTOR_FL_PWM, SAFE_MOTOR_PWM);
  analogWrite(PIN_MOTOR_FR_PWM, SAFE_MOTOR_PWM);
  analogWrite(PIN_MOTOR_RL_PWM, SAFE_MOTOR_PWM);
  analogWrite(PIN_MOTOR_RR_PWM, SAFE_MOTOR_PWM);

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
  // Read output state from global state
  int16_t fl = g_state.output.motor_fl_pwm;
  int16_t fr = g_state.output.motor_fr_pwm;
  int16_t rl = g_state.output.motor_rl_pwm;
  int16_t rr = g_state.output.motor_rr_pwm;

  // --- Update Motor Directions ---
  set_motor_direction(2, fl >= 0);  // FL motor (M3)
  set_motor_direction(3, fr >= 0);  // FR motor (M4)
  set_motor_direction(0, rl >= 0);  // RL motor (M1)
  set_motor_direction(1, rr >= 0);  // RR motor (M2)

  // --- Update Motor PWM (absolute value with bounds check - QA fix: H1) ---
  analogWrite(PIN_MOTOR_FL_PWM, (uint8_t)constrain(abs(fl), MOTOR_PWM_MIN, MOTOR_PWM_MAX));
  analogWrite(PIN_MOTOR_FR_PWM, (uint8_t)constrain(abs(fr), MOTOR_PWM_MIN, MOTOR_PWM_MAX));
  analogWrite(PIN_MOTOR_RL_PWM, (uint8_t)constrain(abs(rl), MOTOR_PWM_MIN, MOTOR_PWM_MAX));
  analogWrite(PIN_MOTOR_RR_PWM, (uint8_t)constrain(abs(rr), MOTOR_PWM_MIN, MOTOR_PWM_MAX));

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
