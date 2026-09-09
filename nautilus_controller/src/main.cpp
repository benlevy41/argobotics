#include <Arduino.h>
#include "control.h"
#include "ManagedMotor.h"
#include "Thruster.h"
#include "hardware_config.h"

// Simple serial control for UUV 

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

    MotorState target; // may not need these any more
    MotorState current;
    Thruster thrusters[static_cast<int>(ThrusterId::COUNT)];
    ManagedMotor servos[static_cast<int>(ServoId::COUNT)];

  public:

    MotorController()

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
        // empty ctor... :(
      }

    void set_up() {
      // set up each thruster and servo - need to call initialize for all motors
      for (Thruster &thruster : thrusters) {
        thruster.initialize();
      }
      for (ManagedMotor &servo : servos) {
        servo.initialize();
      }
    }
    
    void set_state(MotorState state) {
      target = state;
      thrusters[static_cast<int>(ThrusterId::LEFT)].set_target(state.l_thrust);
      thrusters[static_cast<int>(ThrusterId::RIGHT)].set_target(state.r_thrust);
      thrusters[static_cast<int>(ThrusterId::DEPTH1)].set_target(state.depth1);
      thrusters[static_cast<int>(ThrusterId::DEPTH2)].set_target(state.depth2);
      servos[static_cast<int>(ServoId::PAN)].set_target(state.cam_pan);
      servos[static_cast<int>(ServoId::TILT)].set_target(state.cam_tilt);
      delay(3000); // ESC arming delay
    }

    void update() {
      for (Thruster &thruster : thrusters) {
        thruster.update();
      }

      for (ManagedMotor &servo : servos) {
        servo.update();
      }
    }
};

//helper functions

char *read_serial() {
  static char msg[MESSAGE_LENGTH];
  static int msg_pos = 0;

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      msg[msg_pos] = '\0'; //null terminator for c string
      msg_pos = 0;
      return msg;
    }
    else if (msg_pos < MESSAGE_LENGTH - 1) {
      msg[msg_pos] = c;
      msg_pos++;
    }
  }
  return NULL;
}

//runtime objects
static ControlState control_state;
static MotorController motor_control;

void setup() {
  motor_control.set_up();
  Serial.begin(BAUD_RATE);
}

void loop() {
  static unsigned long last_msg_time = 0;
  unsigned long current_time = millis();
  char *received = read_serial();

  if (received != NULL) {
    
    ControlState incoming;
    if (parse_ctrl(received, incoming)) {
      control_state = incoming;
      
      MotorState tgt_state = calc_motor_state(control_state);
      motor_control.set_state(tgt_state);

      last_msg_time = current_time;
    }
    
  }
  else if (current_time - last_msg_time > KEEPALIVE_INTERVAL) {
    motor_control.set_state(MotorState{});
  }

  motor_control.update();
}

