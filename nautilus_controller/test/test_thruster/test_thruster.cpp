#include <unity.h>
#include <Arduino.h>
#include "Thruster.h"

void setUp(void) {

}

void tearDown(void) {

}

void test_thruster_update() {
    Thruster thruster(9, 50, 500);
    thruster.set_target(99); // full forward
    thruster.update();
    TEST_ASSERT_EQUAL(26, thruster.get_pwm());
    thruster.set_target(0); // neutral
    thruster.update();
    TEST_ASSERT_EQUAL(19, thruster.get_pwm());
    thruster.set_target(-99); // full reverse
    thruster.update();
    TEST_ASSERT_EQUAL(13, thruster.get_pwm());
}

void test_thruster_update_with_esc_delay() {
    Thruster thruster(9, 50, 500);
    thruster.set_target(99); // full forward
    thruster.update();
    TEST_ASSERT_EQUAL(26, thruster.get_pwm());
    thruster.set_target(-99); // full reverse
    thruster.update();
    TEST_ASSERT_EQUAL(19, thruster.get_pwm()); // should be neutral due to ESC delay
    delay(600); // wait for ESC delay to pass
    thruster.update();
    TEST_ASSERT_EQUAL(13, thruster.get_pwm()); // now it should be full reverse
}


int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_thruster_update);
    RUN_TEST(test_thruster_update_with_esc_delay);

    UNITY_END();
}