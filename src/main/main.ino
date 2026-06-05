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

#define TILT_Y -200

#define STATUS_BOOT  0
#define STATUS_ALARM 1
#define STATUS_RESETTABLE_ALARM 2
#define STATUS_IDLE 3;


typedef struct context{
  bool tilt;
  bool bump;
  int status;
  unsigned long prevHbSeconds;
  unsigned long HbSeconds;
  unsigned long prevHb100Ms;
  unsigned long Hb100Ms;
  unsigned long prevHb10Ms;
  unsigned long Hb10Ms;

} CONTEXT;

CONTEXT _context;

byte speed;
int dir = 1; 
void stateMachine(CONTEXT* ctx){
  Serial.println(ctx->tilt);
  if(ctx->tilt){
    ctx->status = STATUS_ALARM;
  }
  switch(ctx->status){
    case STATUS_ALARM:
      digitalWrite(MOTOR1_FWD,LOW);
      digitalWrite(MOTOR1_BWD,LOW);
      analogWrite(MOTOR1_SPEED,0);
      digitalWrite(MOTOR2_FWD,LOW);
      digitalWrite(MOTOR2_BWD,LOW);
      analogWrite(MOTOR2_SPEED,0);
      digitalWrite(MOW,LOW);
      //only when we have start button depressed we will accept 
      //a press to exit the alarm status.. button is 1 when depressed
      if(digitalRead(START)==HIGH){
        ctx->status=STATUS_RESETTABLE_ALARM;
      }
    break;
    case STATUS_RESETTABLE_ALARM:
      if(digitalRead(START)==0){
        ctx->status=STATUS_IDLE;
        ctx->tilt=false;
      }
    break;
  }

}
void setup() {
  memset(&_context,0,sizeof(_context));
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
bool accelReadable=false;
void loop() {
  updateHeartBeats(&_context);
  stateMachine(&_context);
  if(_context.HbSeconds>=1){
    accelReadable = true;
  }
  if(accelReadable && _context.Hb100Ms%2 == 0){
    checkTilt(&_context);
  }
  updateHmi(&_context);
  
  
  if(_context.Hb100Ms%2==0){
    consumeAccelerometerInts();

    //Serial.print("L=");
    //Serial.print(analogRead(SENSOR_L));
    //Serial.print("R=");
    //Serial.print(analogRead(SENSOR_R));
   
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
void consumeAccelerometerInts(){
   // must consume interrupts to re-enable
    Wire.beginTransmission(ACCELEROMETER);
    Wire.write(0x30);
    Wire.endTransmission(false);
    Wire.requestFrom(ACCELEROMETER, 1);
    
    if (Wire.available()) {
      byte source = Wire.read();
    }
}
void initAdxl345(){
  
  writeRegister(ACCELEROMETER,0x2D,8); //enable output
  writeRegister(ACCELEROMETER,0x2A,1); //TAP axes=Z
  writeRegister(ACCELEROMETER,0x21,60);//TAP duration 
  writeRegister(ACCELEROMETER,0x2F,0); //int map SINGLE tap on int 1
  writeRegister(ACCELEROMETER,0x1D,0x25); //threeshold tap
  writeRegister(ACCELEROMETER,0x2E,0x40); //int enable single tap
}
void writeRegister(uint8_t paddress,byte regAddress, byte val) {
  Wire.beginTransmission(paddress);
  Wire.write(regAddress);
  Wire.write(val);
  Wire.endTransmission();
}
void checkTilt(CONTEXT *ctx){
    int AX,AY,AZ;
    Wire.beginTransmission(ACCELEROMETER);
    Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(ACCELEROMETER, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
    AX = ( Wire.read() | Wire.read() << 8); // X-axis value
    AY = ( Wire.read() | Wire.read() << 8); // Y-axis value
    AZ = ( Wire.read() | Wire.read() << 8); // Z-axis value
    if(AY>TILT_Y){
      ctx->tilt = true;
    }
}
//interrupt handler for TAP detection
void handleTap(){
  _context.bump=true;
}
void updateHeartBeats(CONTEXT *ctx){
  //heach hb LSB logic vaue has a period of the corresponding duration
  int m = millis();
  if(m-ctx->prevHbSeconds>500){
    ctx->prevHbSeconds=m;
    ctx->HbSeconds++;
  }
  if(m-ctx->prevHb100Ms>50){
    ctx->prevHb100Ms=m;
    ctx->Hb100Ms++;
  }
  if(m-ctx->prevHb10Ms>5){
    ctx->prevHb10Ms=m;
    ctx->Hb10Ms++;
  }

}
void updateHmi(CONTEXT *ctx){
  digitalWrite(BUZZER,ctx->tilt && (ctx->HbSeconds & 1)  ? HIGH:LOW);
}