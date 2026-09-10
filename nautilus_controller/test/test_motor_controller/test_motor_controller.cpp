#include <unity.h>
#include "MotorController.h"
#include "hardware_config.h"

void setUp(void) {
    // Set up before each test
}

void tearDown(void) {
    // Clean up after each test
}

void test_motor_controller_set_state() {
    MotorController motor_controller;
    motor_controller.set_up();

    MotorState state;
    state.l_thrust = 50;
    state.r_thrust = 30;
    state.depth1 = 0;
    state.depth2 = 0;
    state.cam_pan = 90;
    state.cam_tilt = 45;

    motor_controller.set_state(state);
    motor_controller.update(); // Ensure the state is applied

    TEST_ASSERT_EQUAL(22, motor_controller.get_thruster_state(0)); // LEFT
    TEST_ASSERT_EQUAL(21, motor_controller.get_thruster_state(1)); // RIGHT
    TEST_ASSERT_EQUAL(19, motor_controller.get_thruster_state(2)); // DEPTH1
    TEST_ASSERT_EQUAL(19, motor_controller.get_thruster_state(3));   // DEPTH2
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_motor_controller_set_state);

    UNITY_END();
}