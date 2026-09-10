#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include "Thruster.h"
#include "ManagedMotor.h"
#include "control.h"

class MotorController {
private:
    enum class ThrusterId {
        LEFT,
        RIGHT,
        DEPTH1,
        DEPTH2,
        COUNT
    };

    enum class ServoId {
        PAN,
        TILT,
        COUNT
    };

    MotorState state;

    Thruster thrusters[static_cast<int>(ThrusterId::COUNT)];
    ManagedMotor servos[static_cast<int>(ServoId::COUNT)];

public:
    MotorController();

    int get_thruster_state(int ThrusterId);
    void set_up();
    void set_state(MotorState state);
    void update();
};

#endif