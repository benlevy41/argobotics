#include <Arduino.h>
#include "control.h"
#include "ManagedMotor.h"
#include "Thruster.h"
#include "MotorController.h"
#include "hardware_config.h"

// Simple serial control for UUV 

// helper function to read a message from serial input
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

