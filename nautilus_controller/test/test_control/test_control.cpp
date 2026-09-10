#include <unity.h>
#include <cstring>
#include "control.h"
#include "hardware_config.h"


void setUp(void) {
    
}

void tearDown(void) {

}

void test_parse_valid_control_state(void) {
    ControlState test_state;
    char valid_msg[] = "70,-10,0,10,-55";

    bool result = parse_ctrl(valid_msg, test_state);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(70, test_state.throttle);
    TEST_ASSERT_EQUAL_INT(-10, test_state.yaw);
    TEST_ASSERT_EQUAL_INT(0, test_state.heave);
    TEST_ASSERT_EQUAL_INT(10, test_state.pan);
    TEST_ASSERT_EQUAL_INT(-55, test_state.tilt);
}

void test_parse_control_state_short(void) {
    ControlState test_state;
    char short_msg[] = "70,-10,0,10";

    bool result = parse_ctrl(short_msg, test_state);
    TEST_ASSERT_FALSE(result);
}

void test_calc_motor_state_neutral(void) {
    ControlState ctrl;
    MotorState motor = calc_motor_state(ctrl);

    TEST_ASSERT_EQUAL_INT(0, motor.l_thrust);
    TEST_ASSERT_EQUAL_INT(0, motor.r_thrust);
    TEST_ASSERT_EQUAL_INT(0, motor.depth1);
    TEST_ASSERT_EQUAL_INT(0, motor.depth2);
    TEST_ASSERT_EQUAL_INT(0, motor.cam_pan);
    TEST_ASSERT_EQUAL_INT(0, motor.cam_tilt);
}

void test_calc_motor_state_right(void) {
    ControlState ctrl;
    ctrl.throttle = 50;
    ctrl.yaw = -20;
    ctrl.heave = 10;
    ctrl.pan = 5;
    ctrl.tilt = -15;

    MotorState motor = calc_motor_state(ctrl);

    TEST_ASSERT_EQUAL_INT(30, motor.l_thrust);
    TEST_ASSERT_EQUAL_INT(70, motor.r_thrust);
    TEST_ASSERT_EQUAL_INT(10, motor.depth1);
    TEST_ASSERT_EQUAL_INT(10, motor.depth2);
    TEST_ASSERT_EQUAL_INT(5, motor.cam_pan);
    TEST_ASSERT_EQUAL_INT(-15, motor.cam_tilt);
}

void test_calc_motor_state_left(void) {
    ControlState ctrl;
    ctrl.throttle = 50;
    ctrl.yaw = 20;
    ctrl.heave = -10;
    ctrl.pan = -5;
    ctrl.tilt = 15;

    MotorState motor = calc_motor_state(ctrl);

    TEST_ASSERT_EQUAL_INT(70, motor.l_thrust);
    TEST_ASSERT_EQUAL_INT(30, motor.r_thrust);
    TEST_ASSERT_EQUAL_INT(-10, motor.depth1);
    TEST_ASSERT_EQUAL_INT(-10, motor.depth2);
    TEST_ASSERT_EQUAL_INT(-5, motor.cam_pan);
    TEST_ASSERT_EQUAL_INT(15, motor.cam_tilt);
}

void test_motor_state_reverse_left(void) {
    ControlState ctrl;
    ctrl.throttle = -50;
    ctrl.yaw = 20;
    ctrl.heave = 10;
    ctrl.pan = -5;
    ctrl.tilt = 15;

    MotorState motor = calc_motor_state(ctrl);

    TEST_ASSERT_EQUAL_INT(-30, motor.l_thrust);
    TEST_ASSERT_EQUAL_INT(-70, motor.r_thrust);
    TEST_ASSERT_EQUAL_INT(10, motor.depth1);
    TEST_ASSERT_EQUAL_INT(10, motor.depth2);
    TEST_ASSERT_EQUAL_INT(-5, motor.cam_pan);
    TEST_ASSERT_EQUAL_INT(15, motor.cam_tilt);
}

void test_motor_state_reverse_right(void) {
    ControlState ctrl;
    ctrl.throttle = -50;
    ctrl.yaw = -20;
    ctrl.heave = -10;
    ctrl.pan = 5;
    ctrl.tilt = -15;

    MotorState motor = calc_motor_state(ctrl);

    TEST_ASSERT_EQUAL_INT(-70, motor.l_thrust);
    TEST_ASSERT_EQUAL_INT(-30, motor.r_thrust);
    TEST_ASSERT_EQUAL_INT(-10, motor.depth1);
    TEST_ASSERT_EQUAL_INT(-10, motor.depth2);
    TEST_ASSERT_EQUAL_INT(5, motor.cam_pan);
    TEST_ASSERT_EQUAL_INT(-15, motor.cam_tilt);
}

void test_calc_motor_state_constrained(void) {
    ControlState ctrl;
    ctrl.throttle = 100;
    ctrl.yaw = 50;
    ctrl.heave = -150;
    ctrl.pan = 200;
    ctrl.tilt = -300;

    MotorState motor = calc_motor_state(ctrl);

    TEST_ASSERT_EQUAL_INT(99, motor.l_thrust);
    TEST_ASSERT_EQUAL_INT(50, motor.r_thrust);
    TEST_ASSERT_EQUAL_INT(-99, motor.depth1);
    TEST_ASSERT_EQUAL_INT(-99, motor.depth2);
    TEST_ASSERT_EQUAL_INT(99, motor.cam_pan);
    TEST_ASSERT_EQUAL_INT(-99, motor.cam_tilt);
}

void test_read_serial_msg(void) {
    char msg[MESSAGE_LENGTH];
    const char *input = "+70,-10,+00,+10,-55\n";
    bool message_complete = false;

    for (size_t i = 0; i < strlen(input); i++) {
        message_complete = read_serial_msg(input[i], msg);
        if (message_complete) {
            break;
        }
    }

    TEST_ASSERT_TRUE(message_complete);
    TEST_ASSERT_EQUAL_STRING("+70,-10,+00,+10,-55", msg);
}

void test_read_serial_incomplete_msg(void) {
    char msg[MESSAGE_LENGTH];
    const char *input1 = "+70,-10,"; 
    bool message_complete = false;

    for (size_t i = 0; i < strlen(input1); i++) {
        message_complete = read_serial_msg(input1[i], msg);
        if (message_complete) {
            break;
        }
    }

    TEST_ASSERT_FALSE(message_complete);

    const char *input2 = "+00,+10,-55\n";
    for (size_t i = 0; i < strlen(input2); i++) {
        message_complete = read_serial_msg(input2[i], msg);
        if (message_complete) {
            break;
        }
    }

    TEST_ASSERT_TRUE(message_complete);
    TEST_ASSERT_EQUAL_STRING("+70,-10,+00,+10,-55", msg);
}

void test_read_serial_msg_overflow(void) {
    char msg[MESSAGE_LENGTH];
    const char *input = "+70,-10,+00,+10,-55,+99\n"; // 6 values, should overflow
    bool message_complete = false;

    for (size_t i = 0; i < strlen(input); i++) {
        message_complete = read_serial_msg(input[i], msg);
        if (message_complete) {
            break;
        }
    }

    TEST_ASSERT_FALSE(message_complete);

    char input2[MESSAGE_LENGTH] = "+75,+13,-20,+00,-15\n";

    for (size_t i = 0; i < strlen(input2); i++) {
        message_complete = read_serial_msg(input2[i], msg);
        if (message_complete) {
            break;
        }
    }

    TEST_ASSERT_TRUE(message_complete);
    TEST_ASSERT_EQUAL_STRING("+75,+13,-20,+00,-15", msg);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_parse_valid_control_state);
    RUN_TEST(test_parse_control_state_short);
    RUN_TEST(test_calc_motor_state_neutral);
    RUN_TEST(test_calc_motor_state_right);
    RUN_TEST(test_calc_motor_state_left);
    RUN_TEST(test_motor_state_reverse_left);
    RUN_TEST(test_motor_state_reverse_right);
    RUN_TEST(test_calc_motor_state_constrained);
    RUN_TEST(test_read_serial_msg);
    RUN_TEST(test_read_serial_incomplete_msg);
    RUN_TEST(test_read_serial_msg_overflow);

    UNITY_END();
}


