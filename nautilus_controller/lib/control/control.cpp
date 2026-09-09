#include "control.h"
#include <Arduino.h>
#include <stdlib.h>

bool parse_ctrl(char *msg, ControlState &state) {
    int state_arr[5];

    char *tok = strtok(msg, ",");
    int ctr = 0;

    while (tok != NULL) {
        if (ctr >= 5) {
            return false;
        }

        state_arr[ctr] = atoi(tok);
        ctr++;
        tok = strtok(NULL, ",");
    }

    if (ctr != 5) { // invalid packet
        return false;
    }

    state.throttle = state_arr[0];
    state.yaw = state_arr[1];
    state.heave = state_arr[2];
    state.pan = state_arr[3];
    state.tilt = state_arr[4];

    return true;
}

MotorState calc_motor_state(ControlState ctrl) {
    MotorState motor;
    
    motor.l_thrust = ctrl.throttle + ctrl.yaw;
    motor.r_thrust = ctrl.throttle - ctrl.yaw;

    motor.depth1 = ctrl.heave;
    motor.depth2 = ctrl.heave;

    motor.cam_pan = ctrl.pan;
    motor.cam_tilt = ctrl.tilt;

    motor.l_thrust = constrain(motor.l_thrust, -99, 99);
    motor.r_thrust = constrain(motor.r_thrust, -99, 99);
    motor.depth1 = constrain(motor.depth1, -99, 99);
    motor.depth2 = constrain(motor.depth2, -99, 99);
    motor.cam_pan = constrain(motor.cam_pan, -99, 99);
    motor.cam_tilt = constrain(motor.cam_tilt, -99, 99);

    return motor;
}