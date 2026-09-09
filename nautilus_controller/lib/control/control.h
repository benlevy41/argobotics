#ifndef CONTROL_H
#define CONTROL_H

struct ControlState{
    int throttle = 0;
    int yaw = 0;
    int heave = 0;
    int pan = 0;
    int tilt = 0;
};

struct MotorState{
    int l_thrust = 0;
    int r_thrust = 0;
    int depth1 = 0;
    int depth2 = 0;
    int cam_pan = 0;
    int cam_tilt = 0;
};

bool parse_ctrl(char *msg, ControlState &state);

MotorState calc_motor_state(ControlState ctrl);

#endif