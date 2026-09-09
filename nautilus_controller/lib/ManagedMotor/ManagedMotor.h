#ifndef MANAGED_MOTOR_H
#define MANAGED_MOTOR_H

class ManagedMotor {
protected:
    int pin_assignment;
    int pwm_frequency;
    int pwm_signal = 0;
    int current_value = 0;
    int target_value = 0;

    virtual void set_pwm_signal();
    virtual int calc_pwm();

public:
    ManagedMotor(int pin_number, int pwm_freq = 50);

    virtual void update();
    void set_target(int value);
    void initialize();
    int get_pwm();
};

#endif