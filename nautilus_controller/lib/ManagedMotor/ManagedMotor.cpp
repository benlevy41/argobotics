#include "ManagedMotor.h"
#include <cmath>
#include <Arduino.h>

ManagedMotor::ManagedMotor(int pin_number, int pwm_freq)
    : pin_assignment(pin_number), pwm_frequency(pwm_freq)
{
}

void ManagedMotor::set_target(int value) {
    target_value = value;
}

void ManagedMotor::update() {
    if (current_value != target_value) {
        current_value = target_value;
        set_pwm_signal();
    }
}

void ManagedMotor::initialize() {
    analogWriteFrequency(pin_assignment, pwm_frequency);
    current_value = 0;
    set_pwm_signal();
}

void ManagedMotor::set_pwm_signal() {
    pwm_signal = calc_pwm();
    analogWrite(pin_assignment, pwm_signal);
}

int ManagedMotor::calc_pwm() {
    float tgt_time = 1.5 + (current_value / 99.0 * 0.5);
    float duty_time = 1000 / pwm_frequency;
    return std::lround(tgt_time / duty_time * 255);
}

int ManagedMotor::get_pwm() {
    return pwm_signal;
}