// actuators.cpp - Motor, ESC, and servo output implementation
// UpVote Battlebot - Phase 1
#include "actuators.h"
#include "config.h"
#include "state.h"

// ============================================================================
// PRIVATE HELPER FUNCTIONS
// ============================================================================

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
  static uint8_t shift_reg_state = 0x00; // Current shift register state

  // Calculate bit positions for this motor
  uint8_t bit_a = motor * 2;     // Direction A bit
  uint8_t bit_b = motor * 2 + 1; // Direction B bit

  // Set direction bits
  if (forward) {
    shift_reg_state |= (1 << bit_a);   // A = HIGH
    shift_reg_state &= ~(1 << bit_b);  // B = LOW
  } else {
    shift_reg_state &= ~(1 << bit_a);  // A = LOW
    shift_reg_state |= (1 << bit_b);   // B = HIGH
  }

  shift_register_write(shift_reg_state);
}

// ============================================================================
// PUBLIC INTERFACE
// ============================================================================

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

  // --- Weapon ESC Pin ---
  // Note: Phase 1 uses analogWrite() for simplicity
  // Phase 5 will replace with custom 50Hz PWM for better ESC control
  pinMode(PIN_WEAPON_ESC, OUTPUT);
  analogWrite(PIN_WEAPON_ESC, 0);  // Weapon off (will be replaced with proper PWM)

  // --- Self-Righting Servo Pin ---
  // Note: Phase 1 uses analogWrite() for simplicity
  // Phase 6 will add proper Servo library or custom PWM
  pinMode(PIN_SELFRIGHT_SERVO, OUTPUT);
  analogWrite(PIN_SELFRIGHT_SERVO, 0);  // Servo neutral (will be replaced)

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

  // --- Update Motor PWM (absolute value) ---
  analogWrite(PIN_MOTOR_FL_PWM, abs(fl));
  analogWrite(PIN_MOTOR_FR_PWM, abs(fr));
  analogWrite(PIN_MOTOR_RL_PWM, abs(rl));
  analogWrite(PIN_MOTOR_RR_PWM, abs(rr));

  // --- Update Weapon ESC (Phase 5 will improve this) ---
  // For Phase 1, just keep it at safe minimum (0 PWM = stopped)
  analogWrite(PIN_WEAPON_ESC, 0);

  // --- Update Servo (Phase 6 will implement this properly) ---
  // For Phase 1, just keep it at neutral
  analogWrite(PIN_SELFRIGHT_SERVO, 0);
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
