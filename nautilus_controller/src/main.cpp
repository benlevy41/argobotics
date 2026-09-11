#include <Arduino.h>
#include "control.h"
#include "ManagedMotor.h"
#include "Thruster.h"
#include "MotorController.h"
#include "hardware_config.h"

// Simple serial control for UUV 

// helper function to read a message from serial input
char *read_serial(){
  static char msg[MESSAGE_LENGTH];
  bool message_complete = false;
  while (Serial.available()) {
    char c = Serial.read();
    message_complete = read_serial_msg(c, msg);    
    if (message_complete) {
      if (DEBUG_MODE) {
        Serial.println("Complete message: " + String(msg));
      }
      return msg;
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

  Serial.println("Control system active");
  Serial.println("Hardware configuration:");
  Serial.println("Baud rate: " + String(BAUD_RATE));
  Serial.println("Keepalive interval: " + String(KEEPALIVE_INTERVAL) + " ms");
  Serial.println("Left thruster pin: " + String(LEFT_THRUSTER));
  Serial.println("Right thruster pin: " + String(RIGHT_THRUSTER));
  Serial.println("Depth thruster pin 1: " + String(DEPTH_THRUSTER1));
  Serial.println("Depth thruster pin 2: " + String(DEPTH_THRUSTER2));
  Serial.println("Camera pan pin: " + String(PAN_SERVO));
  Serial.println("Camera tilt pin: " + String(TILT_SERVO));
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
      if (DEBUG_MODE) {
        for (int i = 0; i < 4; i++) {
          int val = motor_control.get_thruster_state(i);
          Serial.println("Thruster " + String(i) + " state: " + String(val));
        }
      }
      last_msg_time = current_time;
    }
  }
  
  else if (current_time - last_msg_time > KEEPALIVE_INTERVAL) {
    motor_control.set_state(MotorState{});
  }

  motor_control.update();
}

