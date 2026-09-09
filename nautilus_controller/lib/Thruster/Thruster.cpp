#include "Thruster.h"
#include <Arduino.h>

Thruster::Thruster(int pin_number, int pwm_freq, int esc_switching_delay)
    : ManagedMotor(pin_number, pwm_freq)
{
    this->esc_switching_delay = esc_switching_delay;
}

void Thruster::update() {
    if (current_value == target_value) {
        return;
    }

    if (timer_start != 0) {
        unsigned long now = millis();

        if (now - timer_start > esc_switching_delay) {
            timer_start = 0;
            current_value = target_value;
            set_pwm_signal();
        }

        return;
    }

    if ((target_value < 0 && current_value > 0) ||
        (target_value > 0 && current_value < 0)) {

        current_value = 0;
        timer_start = millis();
        set_pwm_signal();
        return;
    }

    current_value = target_value;
    set_pwm_signal();
}