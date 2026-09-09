#include "MotorController.h"
#include "hardware_config.h"
#include <Arduino.h>

MotorController::MotorController()
    : thrusters{
        Thruster(LEFT_THRUSTER),
        Thruster(RIGHT_THRUSTER),
        Thruster(DEPTH_THRUSTER1),
        Thruster(DEPTH_THRUSTER2)
      },
      servos{
        ManagedMotor(PAN_SERVO),
        ManagedMotor(TILT_SERVO)
      }
{
}

void MotorController::set_up() {
    for (Thruster &thruster : thrusters) {
        thruster.initialize();
    }
    for (ManagedMotor &servo : servos) {
        servo.initialize();
    }
}

void MotorController::set_state(MotorState incoming) {
    this->state = incoming;
    thrusters[static_cast<int>(ThrusterId::LEFT)].set_target(state.l_thrust);
    thrusters[static_cast<int>(ThrusterId::RIGHT)].set_target(state.r_thrust);
    thrusters[static_cast<int>(ThrusterId::DEPTH1)].set_target(state.depth1);
    thrusters[static_cast<int>(ThrusterId::DEPTH2)].set_target(state.depth2);

    servos[static_cast<int>(ServoId::PAN)].set_target(state.cam_pan);
    servos[static_cast<int>(ServoId::TILT)].set_target(state.cam_tilt);

    delay(3000); // ESC arming delay
}

void MotorController::update() {
    for (Thruster &thruster : thrusters) {
        thruster.update();
    }

    for (ManagedMotor &servo : servos) {
        servo.update();
    }
}