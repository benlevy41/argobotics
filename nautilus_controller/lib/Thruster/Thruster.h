#ifndef THRUSTER_H
#define THRUSTER_H

#include "ManagedMotor.h"

class Thruster : public ManagedMotor {
private:
    unsigned long timer_start = 0;
    int esc_switching_delay;

public:
    Thruster(int pin_number, int pwm_freq = 50, int esc_switching_delay = 500);

    void update() override;
};

#endif