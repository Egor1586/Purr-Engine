#include <QTRSensors.h>

QTRSensors qtr;
const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

const int AIN1 = 6;
const int AIN2 = 9;
const int BIN1 = 11;
const int BIN2 = 10;
const int STBY = 8;
const int LIGHT = 12;

const int RED = 7;
const int GREEN = 5;
const int BLUE = 3;

const int startButton= 3;

bool isRunning = false;
int lastError = 0;
int integral = 0;

const int baseSpeedL = 120;
const int baseSpeedR = 120;
float Kp = 0.053;
float Ki = 0;
float Kd = 0.69;

int stopCount = 0;
int crossLineCount = 0;
const int blackLinekLim = 800;
const int minPosition = 300;
const int maxPosition = 6700;
const int stopLimit = 20;
const int countStopLine = 2;

void setup() {
  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]){A0, A1, A2, A3, A4, A5, A6, A7}, SensorCount);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);

  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(LIGHT, OUTPUT);

  digitalWrite(STBY, HIGH);
  digitalWrite(LIGHT, HIGH);

  pinMode(startButton, INPUT_PULLUP);
  
  digitalWrite(GREEN, HIGH);
  calibrateSensors();
  digitalWrite(GREEN, LOW);
}

void calibrateSensors() {
  delay(1000);

  for (uint16_t i = 0; i < 300; i++) {
    qtr.calibrate();
    delay(15);
  }
}

void loop() {
  handleStartStopButton(); 
  
  if (isRunning) {
    uint16_t position = qtr.readLineBlack(sensorValues);

    bool blackLine = true;
    for (int i = 0; i < SensorCount; i++) {
      if (sensorValues[i] < blackLinekLim) { 
        blackLine = false;
        break;
      }
    }

    static bool wasBlackLine = false;
    if (blackLine && !wasBlackLine) {
      crossLineCount++;
      wasBlackLine = true;
    }
    if (!blackLine) {
      wasBlackLine = false;
    }

    if (crossLineCount >= countStopLine) {
      stopMotors();
      isRunning = false;
      crossLineCount = 0;
      return;
    }

    // if (position <= minPosition || position >= maxPosition) {
    //   stopCount++;
    // } 
    // else {
    //   stopCount = 0;
    // }

    // if (stopCount >= stopLimit) {
    //  stopMotors();
    //  isRunning = false;
    //  return;
    // }

    int error = position - 3500;

    int P = error;
    integral += error;
    if (integral > 1000) integral = 1000;
    if (integral < -1000) integral = -1000;
    int I = integral;
    int D = error - lastError;
    lastError = error;

    int pidValue = Kp * P + Ki * I + Kd * D;
    int leftMotorSpeed = baseSpeedL - pidValue;
    int rightMotorSpeed = baseSpeedR + pidValue;

    leftMotorSpeed = constrain(leftMotorSpeed, 0, 255);
    rightMotorSpeed = constrain(rightMotorSpeed, 0, 255);
  
    driveMotors(leftMotorSpeed, rightMotorSpeed);

  } 
  else {
    digitalWrite(GREEN, LOW);
    digitalWrite(RED, HIGH);
    stopMotors();
  }
}

void handleStartStopButton() {
  static unsigned long lastPress = 0;
  if (digitalRead(startButton) == LOW && millis() - lastPress > 500) {
    lastPress = millis();
    
    if (isRunning) {
      isRunning = false;
      digitalWrite(RED, HIGH);
      digitalWrite(GREEN, LOW);
    }
    else {
      isRunning= true;
      delay(2000);
      digitalWrite(GREEN, HIGH);
      digitalWrite(RED, LOW);
    }
  }
}

void driveMotors(int leftMotorSpeed, int rightMotorSpeed) {
  analogWrite(AIN2, leftMotorSpeed);

  analogWrite(BIN1, rightMotorSpeed);
}

void stopMotors() {
  analogWrite(AIN2, 0);
  analogWrite(BIN1, 0);
}




