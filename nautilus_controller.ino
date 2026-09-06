// Simple serial control for UUV 

#define DEPTH_THRUSTER1 2
#define DEPTH_THRUSTER2 3
#define RIGHT_THRUSTER 4
#define LEFT_THRUSTER 5
#define TILT_SERVO 9
#define PAN_SERVO 10

String readString;

struct Inst{
  char dir;
  int s; // -9-9: 0 is neutral(stopped)
};

struct Inst readInst(String str) {
  struct Inst inst;
  inst.dir = str.charAt(0);
  inst.s = str.substring(1,3).toInt();
  return inst;
}

int convert(int s){
  // at 500hz, 1.5ms (neutral) is .75 duty cycle, 1ms (full rev) is .50 duty cycle, and 2ms (full fwd) is 1.00 duty cycle
  // scale speed of -9 to 9 to duty cycle between 0.5 - 1.0 and convert to 8 bit integer representation
  if (s > 9){s = 9;}
  if (s < -9){s=-9;}
  s = s + 9;
  float fs = float(s)/36 + 0.5;
  return round(255*fs);
}

void thrust_test(int pin){
  analogWrite(pin,0); //no write
  delay(3000);
  analogWrite(pin,191); //neutral
  delay(3000);
  analogWrite(pin,210); //slow fwd
  delay(3000);
  analogWrite(pin,255); //fast fwd
  delay(3000);
  analogWrite(pin,191); //stop
  delay(1000);
  analogWrite(pin,172); //slow rev
  delay(3000);
  analogWrite(pin,128); //full rev
  delay(3000);
  analogWrite(pin,191); //stop
  analogWrite(pin,0); //no write
}

void stop(){
  analogWrite(LEFT_THRUSTER, 191);//convert(0));
  analogWrite(RIGHT_THRUSTER, 191);//convert(0));
  analogWrite(DEPTH_THRUSTER1, 191);//convert(0));
  analogWrite(DEPTH_THRUSTER2, 191);
}

void pan(int s){
  analogWrite(PAN_SERVO, convert(s));
}

void tilt(int s){
  analogWrite(TILT_SERVO, convert(s));
}

void up(int s){
  analogWrite(DEPTH_THRUSTER1, convert(0));
  analogWrite(DEPTH_THRUSTER2, convert(0));
  delay(300);
  analogWrite(DEPTH_THRUSTER1, convert(s));
  analogWrite(DEPTH_THRUSTER2, convert(s));
}

void fwd(int s){
  analogWrite(LEFT_THRUSTER, convert(s));
  analogWrite(RIGHT_THRUSTER, convert(s));
}

void left(int s){
  //for turning, negative s uses a reverse/fwd to pivot in place, positive s uses neutral/fwd for a more gentle forward turn
  analogWrite(LEFT_THRUSTER, convert(0));
  analogWrite(RIGHT_THRUSTER, convert(0));
  delay(300);

  if(s<0){
    analogWrite(LEFT_THRUSTER, convert(s));
    analogWrite(RIGHT_THRUSTER, convert(-s));
  }
  else{
    analogWrite(LEFT_THRUSTER, convert(0));
    analogWrite(RIGHT_THRUSTER, convert(s));
  }
}

void right(int s){
  analogWrite(LEFT_THRUSTER, convert(0));
  analogWrite(RIGHT_THRUSTER, convert(0));
  delay(300);
  if(s<0){
    analogWrite(LEFT_THRUSTER, convert(-s));
    analogWrite(RIGHT_THRUSTER, convert(s));
  }
  else{
    analogWrite(LEFT_THRUSTER, convert(s));
    analogWrite(RIGHT_THRUSTER, convert(0));
  }
}

void setup() {
  //set pwm pin frequency to 500hz
  analogWriteFrequency(DEPTH_THRUSTER1, 500);
  analogWriteFrequency(DEPTH_THRUSTER2, 500);
  analogWriteFrequency(RIGHT_THRUSTER, 500);
  analogWriteFrequency(LEFT_THRUSTER, 500);
  analogWriteFrequency(PAN_SERVO, 500);
  analogWriteFrequency(TILT_SERVO, 500);
  delay(5000);
  //unlock esc by setting pwm signal to 1.5ms/.75duty cycle for 3 sec
  analogWrite(DEPTH_THRUSTER1, 191);
  analogWrite(DEPTH_THRUSTER2, 191);
  analogWrite(RIGHT_THRUSTER, 191);
  analogWrite(LEFT_THRUSTER,191);
  delay(3000);
  //initialize serial bus
  Serial.begin(9600);
}

void loop() {

  while (Serial.available()) {
    delay(2);  //delay to allow byte to arrive in input buffer
    char c = Serial.read();
    readString += c;
  }

  if (readString.length() >0) {
    struct Inst instruct = readInst(readString);
    String response = instruct.dir + "\t" + String(instruct.s);
    Serial.println(response);

    switch(instruct.dir){
      case 'u': up(instruct.s);
        break;
      case 'x': stop();
        break;
      case 'f': fwd(instruct.s);
        break;
      case 'l': left(instruct.s);
        break;
      case 'r': right(instruct.s);
        break;
      case 'p': pan(instruct.s);
        break;
      case 't': tilt(instruct.s);
        break;
      default: break;
    }
    readString="";
  } 
}

