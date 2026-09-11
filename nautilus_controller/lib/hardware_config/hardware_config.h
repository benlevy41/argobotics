#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

//pin assignments
const int DEPTH_THRUSTER1 = 2;
const int DEPTH_THRUSTER2 = 3;
const int RIGHT_THRUSTER = 4;
const int LEFT_THRUSTER = 5;
const int TILT_SERVO = 9;
const int PAN_SERVO = 10;

//other constants
const int MESSAGE_LENGTH = 20;
const long KEEPALIVE_INTERVAL = 500;
const int BAUD_RATE = 9600;
const int PWM_FREQ = 50;
const int ESC_SWITCHING_DELAY = 500;
const bool DEBUG_MODE = false;
#endif // HARDWARE_CONFIG_H