#include <Arduino.h>
#include "control.h"
#include "ManagedMotor.h"

// Simple serial control for UUV 

//pin assignments
const int DEPTH_THRUSTER1 = 2;
const int DEPTH_THRUSTER2 = 3;
const int RIGHT_THRUSTER = 4;
const int LEFT_THRUSTER = 5;
const int TILT_SERVO = 9;
const int PAN_SERVO = 10;

//other constants
const long KEEPALIVE_INTERVAL = 500;
const int MESSAGE_LENGTH = 5;
const int BAUD_RATE = 9600;
const int PWM_FREQ = 50;
const int ESC_SWITCHING_DELAY = 500;


//class definitions
/*
class ManagedMotor {
  protected:
    int current_value;  // -99 to 99
    int target_value;   // -99 to 99
    int pin_assignment;

    void set_pwm_signal() {
      float tgt_time = 1.5 + (current_value / 99.0 * 0.5); // scaled from 1.0 to 2.0 ms
      float duty_time = 1000 / PWM_FREQ;
      int pwm_signal = round(tgt_time / duty_time * 255); // scaled from 0 to 255
      analogWrite(pin_assignment, pwm_signal);
    }
  
  public:
    ManagedMotor(int pin_number) {
      pin_assignment = pin_number;
    }

    void set_target(int value) {
      target_value = constrain(value, -99, 99);
    }

    virtual void update() {
      if (current_value != target_value){ 
        current_value = target_value;
        set_pwm_signal();
        return;
      } 
    }

    void initialize() {
      analogWriteFrequency(pin_assignment, PWM_FREQ);
      current_value = 0;
      set_pwm_signal();
    }
};
*/

class Thruster : public ManagedMotor {
  private:
    unsigned long timer_start = 0;

  public:
    Thruster(int pin_number, int pwm_freq = 50)
      : ManagedMotor(pin_number, pwm_freq)
      {
        // empty ctor... :(
      }

    void update() override {
      if (current_value == target_value){ return; } // no action needed

      if (timer_start != 0) { // timer was started - ESC should be in idle but may be used for smooth acceleration later
        unsigned long now = millis();
        if (now - timer_start > ESC_SWITCHING_DELAY) {  // timer complete
          timer_start = 0;
          current_value = target_value;
          set_pwm_signal();
          return;
        } else {
          return;
        }
      }

      if ((target_value < 0 && current_value > 0) || (target_value > 0 && current_value < 0)) {  // idle ESC to prevent damage
        current_value = 0;
        timer_start = millis();
        set_pwm_signal();
        return;
      }

      current_value = target_value;
      set_pwm_signal();
      return;
    }
};

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

