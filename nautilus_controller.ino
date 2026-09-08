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
const int MESSAGE_LENGTH = 16;
const int BAUD_RATE = 9600;
const int PWM_FREQ = 50;
const int ESC_SWITCHING_DELAY = 500;

//struct definitions

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

class Thruster {
  private:
    int current_value;  // -99 to 99
    int target_value;   // -99 to 99
    int pin_assignment;
    unsigned long timer_start = 0;

    void set_thrust() {
      if (input > 99){input = 99;}
      if (input < -99){input=-99;}
      float tgt_time = 1.5 + (input / 99.0 * 0.5); // scaled from 1.0 to 2.0 ms
      float duty_time = 1000 / PWM_FREQ;
      pwm_signal = round(tgt_time / duty_time * 255); // scaled from 0 to 255
      analogWrite(pin_assignment, pwm_signal);
    }

  public:
    void set_target(int value) {
      target_value = value;
    }

    void update() {
      if (current_value == target_value){ return; } // no action needed

      if (timer_start != 0) { // timer was started - ESC should be in idle but may be used for smooth acceleration later
        unsigned long now = millis();
        if (now - timer_start > ESC_SWITCHING_DELAY) {  // timer complete
          timer_start = 0;
          current_value = target_value;
          set_thrust();
          return;
        } else {
          return;
        }
      }

      if ((target_value < 0 && current_value > 0) || (target_value > 0 && current_value < 0)) {  // idle ESC to prevent damage
        current_value = 0;
        timer_start = millis();
        set_thrust();
        return;
      }

      current_value = target_value;
      set_thrust();
      return;
    }

    void initialize() {
      analogWriteFrequency(pin_assignment, PWM_FREQ);
      current_value = 0;
      set_thrust();
    }
};

class MotorController {

  private: 
    MotorState target;
    MotorState current;

    int safe_transition(int i_current, int i_target) {
      if ((i_current < 0 && i_target > 0) || (i_current > 0 && i_target < 0)) {
        return 0;
      } else {
        return i_target;
      }
    }

    int get_pwm_val(int input) {
      // pulse width = 1000ms / pwm_freq 
      // 1.0ms = full rev. / 0deg | 1.5ms = neutral / 90deg | 2.0ms = full fwd. / 180deg
      // teensy analogwrite takes duty cycle values 0-255
      if (input > 99){input = 99;}
      if (input < -99){input=-99;}
      float tgt_time = 1.5 + (input / 99.0 * 0.5); // scaled from 1.0 to 2.0 ms
      float duty_time = 1000 / PWM_FREQ;
      return round(tgt_time / duty_time * 255); // scaled from 0 to 255
    }

  public:
    void set_state(MotorState state) {
      target = state;
    }

    void update() {
      if (current.l_thrust != target.l_thrust) {
        current.l_thrust = safe_transition(current.l_thrust, target.l_thrust);
        analogWrite(LEFT_THRUSTER, get_pwm_val(current.l_thrust));
      }
      if (current.r_thrust != target.r_thrust) {
        current.r_thrust = safe_transition(current.r_thrust, target.r_thrust);
        analogWrite(RIGHT_THRUSTER, get_pwm_val(current.r_thrust));
      }
      if (current.depth1 != target.depth1) {
        current.depth1 = safe_transition(current.depth1, target.depth1);
        analogWrite(DEPTH_THRUSTER1, get_pwm_val(current.depth1));
      }
      if (current.depth2 != target.depth2) {
        current.depth2 = safe_transition(current.depth2, target.depth2);
        analogWrite(DEPTH_THRUSTER2, get_pwm_val(current.depth2));
      }
      if (current.cam_pan != target.cam_pan) {
        current.cam_pan = target.cam_pan;
        analogWrite(PAN_SERVO, get_pwm_val(current.cam_pan));
      }
      if (current.cam_tilt != target.cam_tilt) {
        current.cam_tilt = target.cam_tilt;
        analogWrite(TILT_SERVO, get_pwm_val(current.cam_tilt));
      }
    }

    void init_thrusters() {
      //set pwm pin frequency
      analogWriteFrequency(DEPTH_THRUSTER1, PWM_FREQ);
      analogWriteFrequency(DEPTH_THRUSTER2, PWM_FREQ);
      analogWriteFrequency(RIGHT_THRUSTER, PWM_FREQ);
      analogWriteFrequency(LEFT_THRUSTER, PWM_FREQ);
      analogWriteFrequency(PAN_SERVO, PWM_FREQ);
      analogWriteFrequency(TILT_SERVO, PWM_FREQ);
      delay(5000);

      //unlock esc by setting pwm signal to 1.5ms cycle for 3 sec
      analogWrite(DEPTH_THRUSTER1, get_pwm_val(0));
      analogWrite(DEPTH_THRUSTER2, get_pwm_val(0));
      analogWrite(RIGHT_THRUSTER, get_pwm_val(0));
      analogWrite(LEFT_THRUSTER, get_pwm_val(0));
      delay(3000);
    }
};

struct ControlState parse_ctrl(char *str) {
  struct ControlState state;
  int state_arr[MESSAGE_LENGTH];

  char *tok = strtok(str, ",");
  int ctr = 0;

  while (tok != NULL && ctr < MESSAGE_LENGTH) {
    state_arr[ctr] = atoi(tok);
    ctr++;
    tok = strtok(NULL, ",");
  }

  state.throttle = state_arr[0];
  state.yaw = state_arr[1];
  state.heave = state_arr[2];
  state.pan = state_arr[3];
  state.tilt = state_arr[4];

  return state
}

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
    else if (msg_pos < MESSAGE_LENGTH) {
      msg[msg_pos] = c;
      msg_pos++;
    }
  }
  return NULL;
}

MotorState calc_motor_state(ControlState ctrl) {
  MotorState motor;
  
  motor.l_thrust = ctrl.throttle + ctrl.yaw;
  motor.r_thrust = ctrl.throttle - ctrl.yaw;

  motor.depth1 = ctrl.heave;
  motor.depth2 = ctrl.heave;

  motor.l_thrust = constrain(motor.l_thrust, -99, 99);
  motor.r_thrust = constrain(motor.r_thrust, -99, 99);
  motor.depth1 = constrain(motor.depth1, -99, 99);
  motor.depth2 = constrain(motor.depth2, -99, 99);

  return motor;
}

static ControlState control_state;
static MotorController motor_control;

void setup() {
  motor_control.init_thrusters();
  Serial.begin(BAUD_RATE);
}


void loop() {
  static unsigned long last_msg_time = 0;
  unsigned long current_time = millis();
  char *received = read_serial();

  if (received != NULL) {
    last_msg_time = current_time;
    control_state = parse_ctrl(received);
    MotorState tgt_state = calc_motor_state(control_state);
    motor_control.set_state(tgt_state);
  }
  else if (current_time - last_msg_time > KEEPALIVE_INTERVAL) {
    motor_control.set_state(MotorState{});
  }

  motor_control.update();
}

