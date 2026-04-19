#include <Arduino.h>
#include <Bluepad32.h>
#include <stdbool.h>

const int HP = 25;
const int DL = 27;
const int HL = 14;
const int DP = 13;
const int steerMaster = 33;

const int forwardChannel = 0;
const int reverseChannel = 1;
const int servoChannel = 2;

GamepadPtr myGamepads[BP32_MAX_GAMEPADS];

void TimeToAbandonTheShip(void){
  digitalWrite(HP, 1);
  digitalWrite(HL, 1);
  ledcWrite(forwardChannel, 0);
  ledcWrite(reverseChannel, 0);
  ledcWrite(servoChannel, 4551);
}

void onConnectedGamepad(GamepadPtr clanker) {
    bool jackPot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myGamepads[i] == NULL) {
            printf("Xbox Controller Connected (slot: %d\n)", i);
            myGamepads[i] = clanker;
            jackPot = true;
            break;
        }
    }
    if (!jackPot) {
        printf("Hueston, we got a problem! ");
    }
}

void onDisconnectedGamepad(GamepadPtr clanker) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myGamepads[i] == clanker) {
            printf("We are cooked (stage1) (Slot: %d\n)", i);
            myGamepads[i] = NULL;
            TimeToAbandonTheShip();
            break;
        }
    }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(HP, OUTPUT);
  pinMode(HL, OUTPUT);
  pinMode(DP, OUTPUT);
  pinMode(DL, OUTPUT);

  ledcSetup(reverseChannel, 5000, 8);
  ledcSetup(forwardChannel, 5000, 8);
    
  ledcAttachPin(DL, reverseChannel);
  ledcAttachPin(DP, forwardChannel);

  pinMode(steerMaster, OUTPUT);
  ledcSetup(servoChannel, 50, 16);
  ledcAttachPin(steerMaster, servoChannel);

  TimeToAbandonTheShip();
  //digitalWrite(HP, 0);
  BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
}

void loop() {
  BP32.update();

  float speed = 0;
  float direction = 90;

  int i = 0;
  GamepadPtr clanker = myGamepads[i];
  if(clanker && clanker->isConnected()){

    if(clanker->throttle()){
      speed += ((clanker->throttle())*256)/1023;
    }
    if(clanker->brake()){
      speed -= ((clanker->brake())*256)/1023;
    }
    if(speed > 0){
      digitalWrite(HP, 1);
      ledcWrite(reverseChannel, 0);
      delay(10);
      digitalWrite(HL, 0);
      ledcWrite(forwardChannel, speed);
      printf("We going places (%f)", speed*100/256);
    }
    else if(speed < 0){
      digitalWrite(HL, 1);
      ledcWrite(forwardChannel, 0);
      delay(10);
      int reverseSpeed = abs(speed);
      digitalWrite(HP, 0);
      ledcWrite(reverseChannel, reverseSpeed);
      printf("Moonwalk baby! (%f)", speed*100/256);
    }
    else TimeToAbandonTheShip();

    int x = clanker->axisRX();
    if(x < 40 && x > -40) ledcWrite(servoChannel, 4551);//combating stick drift mthfckrs
    else {
    int direction = 4915 + (x * 1638 / 512 / 2); 
    
    ledcWrite(servoChannel, direction);
    printf("Steermaster is going %d ", direction);
    }
  }
}
