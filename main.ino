#include <Wire.h>

#define ACCELEROMETER 0x53

#define BUZZER 11
#define MOW    14
#define MOTOR1_FWD 3
#define MOTOR1_BWD 4
#define MOTOR1_SPEED 5
#define MOTOR2_FWD 7
#define MOTOR2_BWD 8
#define MOTOR2_SPEED 6
#define START 9
#define HOME 10
#define ON_RED 12
#define ON_GREEN 13
#define SENSOR_R A3
#define SENSOR_L A2
#define TAP 2

unsigned long prevTime;
byte speed;
int dir = 1; 

float X_out, Y_out, Z_out;  // Outputs
float X1, Y1, Z1;

void setup() {
  Wire.begin();
  initAdxl345();
  Serial.begin(115200);
  pinMode(BUZZER,OUTPUT);
  pinMode(MOW,OUTPUT);
  pinMode(MOTOR1_FWD,OUTPUT);
  pinMode(MOTOR1_SPEED,OUTPUT);
  pinMode(MOTOR1_BWD,OUTPUT);
  pinMode(MOTOR2_FWD,OUTPUT);
  pinMode(MOTOR2_SPEED,OUTPUT);
  pinMode(MOTOR2_BWD,OUTPUT);
  pinMode(START,INPUT_PULLUP);
  pinMode(HOME,INPUT_PULLUP);
  pinMode(ON_RED,OUTPUT);
  pinMode(ON_GREEN,OUTPUT);
  pinMode(TAP, INPUT);
  attachInterrupt(digitalPinToInterrupt(TAP), handleTap, RISING);
}

void loop() {
  
  
  if(millis()-prevTime>100){
    prevTime=millis();
     // must consume interrupts to re-enable
    Wire.beginTransmission(ACCELEROMETER);
    Wire.write(0x30);
    Wire.endTransmission(false);
    Wire.requestFrom(ACCELEROMETER, 1);
    
    if (Wire.available()) {
      byte source = Wire.read();
    }


    Wire.beginTransmission(ACCELEROMETER);
    Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(ACCELEROMETER, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
    X_out = ( Wire.read() | Wire.read() << 8); // X-axis value
    X1 = X_out / 256; //For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
    Y_out = ( Wire.read() | Wire.read() << 8); // Y-axis value
    Y1 = Y_out / 256;
    Z_out = ( Wire.read() | Wire.read() << 8); // Z-axis value
    Z1 = Z_out / 256;
    Serial.print("L=");
    Serial.print(analogRead(SENSOR_L));
    Serial.print("R=");
    Serial.print(analogRead(SENSOR_R));
    Serial.print("AX=");
    Serial.print(X1);
    Serial.print("AY=");
    Serial.print(Y1);
    Serial.print("AZ=");
    Serial.println(Z1);
    
  }
  /*
  if(millis()-prevTime>10){
    if(dir>0){
      digitalWrite(MOTOR2_FWD,HIGH);
      digitalWrite(MOTOR2_BWD,LOW);
    }else{
      digitalWrite(MOTOR2_FWD,LOW);
      digitalWrite(MOTOR2_BWD,HIGH);
    }
    prevTime=millis();
    speed+=dir;
    if(speed==255 || speed==0){
      dir=-dir;
    }
    analogWrite(MOTOR2_SPEED,speed);
    
  }*/
  
}
void initAdxl345(){
  
  writeRegister(0x2D,8); //enable output
  writeRegister(0x2A,1); //TAP axes=Z
  writeRegister(0x21,60);//TAP duration 
  writeRegister(0x2F,0); //int map SINGLE tap on int 1
  writeRegister(0x1D,0x25); //threeshold tap
  writeRegister(0x2E,0x40); //int enable single tap
}
void writeRegister(byte regAddress, byte val) {
  Wire.beginTransmission(ACCELEROMETER);
  Wire.write(regAddress);
  Wire.write(val);
  Wire.endTransmission();
}

void handleTap(){
  digitalWrite(BUZZER,!digitalRead(BUZZER));
}