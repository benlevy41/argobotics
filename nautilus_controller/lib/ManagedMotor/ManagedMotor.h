#ifndef MANAGED_MOTOR_H
#define MANAGED_MOTOR_H

class ManagedMotor {
protected:
    int pwm_frequency;
    int current_value = 0;
    int target_value = 0;
    int pin_assignment;

    virtual void set_pwm_signal();

public:
    ManagedMotor(int pin_number, int pwm_freq = 50);

    virtual void update();
    void set_target(int value);
    void initialize();
};

#endif