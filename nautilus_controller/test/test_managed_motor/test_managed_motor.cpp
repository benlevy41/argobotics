#include <unity.h>
#include <Arduino.h>
#include "ManagedMotor.h"

void setUp(void) {

}

void tearDown(void) {

}

void test_managed_motor_calc_pwm_signal(void) {
    ManagedMotor motor(9, 50);
    motor.set_target(99); // full forward
    motor.update();
    TEST_ASSERT_EQUAL(26, motor.get_pwm());
    motor.set_target(0); // neutral
    motor.update();
    TEST_ASSERT_EQUAL(19, motor.get_pwm());
    motor.set_target(-99); // full reverse
    motor.update();
    TEST_ASSERT_EQUAL(13, motor.get_pwm());
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_managed_motor_calc_pwm_signal);

    UNITY_END();
}