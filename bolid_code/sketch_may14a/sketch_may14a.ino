#include <QTRSensors.h>

QTRSensors qtr;
const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

// Пины моторов
#define LEFT_MOTOR_PIN1 2
#define LEFT_MOTOR_PIN2 3
#define RIGHT_MOTOR_PIN1 4
#define RIGHT_MOTOR_PIN2 5
#define LEFT_MOTOR_PWM 9
#define RIGHT_MOTOR_PWM 10

#define START_BUTTON 7

// Переменные
bool isRunning = false;
int lastError = 0;
int integral = 0;
int currentLeftSpeed = 0;
int currentRightSpeed = 0;

const int BASE_SPEED = 150;
float Kp = 0.4, Ki = 0.01, Kd = 0.3; // Подстроенные PID-параметры

void setup() {
  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]){A0, A1, A2, A3, A4, A5, A6, A7}, SensorCount);

  pinMode(LEFT_MOTOR_PIN1, OUTPUT);
  pinMode(LEFT_MOTOR_PIN2, OUTPUT);
  pinMode(RIGHT_MOTOR_PIN1, OUTPUT);
  pinMode(RIGHT_MOTOR_PIN2, OUTPUT);
  pinMode(LEFT_MOTOR_PWM, OUTPUT);
  pinMode(RIGHT_MOTOR_PWM, OUTPUT);
  pinMode(START_BUTTON, INPUT_PULLUP);

  Serial.begin(9600);
  waitForCalibration();
}

void waitForCalibration() {
  Serial.println("Нажмите кнопку для калибровки...");
  while (digitalRead(START_BUTTON) == HIGH) delay(10);
  calibrateSensors();
}

void calibrateSensors() {
  Serial.println("Калибровка...");
  pinMode(LED_BUILTIN, OUTPUT);
  for (uint16_t i = 0; i < 400; i++) {
    if (i % 40 == 0) digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    qtr.calibrate();
    delay(25);
  }
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("Калибровка завершена.");
  while (digitalRead(START_BUTTON) == LOW) delay(10);
  while (digitalRead(START_BUTTON) == HIGH) delay(10);
  delay(1000);
  isRunning = true;
  Serial.println("Старт!");
}

void loop() {
  handleStartStopButton();

  if (isRunning) {
    uint16_t position = qtr.readLineBlack(sensorValues);

    // Обработка потери линии (0-7000)
    if (position <= 200 || position >= 6800) {
      Serial.println("Линия потеряна!");
      smoothStop();
      return;
    }

    int error = position - 3500; // Центр для 8 сенсоров: 0-7000
    Serial.print("Позиция: "); Serial.print(position);
    Serial.print(" Ошибка: "); Serial.println(error);

    int P = error;
    integral += error;
    integral = constrain(integral, -1000, 1000);
    int I = integral;
    int D = error - lastError;
    lastError = error;

    int pidValue = Kp * P + Ki * I + Kd * D;

    int leftSpeed = BASE_SPEED - pidValue;
    int rightSpeed = BASE_SPEED + pidValue;
    constrainSpeeds(leftSpeed, rightSpeed);

    smoothDrive(leftSpeed, rightSpeed);
  } else {
    smoothStop();
  }

  delay(10);
}

void handleStartStopButton() {
  static unsigned long lastPress = 0;
  if (digitalRead(START_BUTTON) == LOW && millis() - lastPress > 500) {
    lastPress = millis();
    isRunning = !isRunning;
    Serial.println(isRunning ? "Старт!" : "Стоп!");
  }
}

void constrainSpeeds(int &left, int &right) {
  left = constrain(left, 0, 255);
  right = constrain(right, 0, 255);
}

void smoothDrive(int targetLeft, int targetRight) {
  int step = 5;
  if (currentLeftSpeed < targetLeft) currentLeftSpeed += step;
  else if (currentLeftSpeed > targetLeft) currentLeftSpeed -= step;

  if (currentRightSpeed < targetRight) currentRightSpeed += step;
  else if (currentRightSpeed > targetRight) currentRightSpeed -= step;

  driveMotors(currentLeftSpeed, currentRightSpeed);
}

void smoothStop() {
  if (currentLeftSpeed > 0) currentLeftSpeed -= 5;
  if (currentRightSpeed > 0) currentRightSpeed -= 5;
  if (currentLeftSpeed < 0) currentLeftSpeed = 0;
  if (currentRightSpeed < 0) currentRightSpeed = 0;
  driveMotors(currentLeftSpeed, currentRightSpeed);
}

void driveMotors(int leftSpeed, int rightSpeed) {
  digitalWrite(LEFT_MOTOR_PIN1, HIGH);
  digitalWrite(LEFT_MOTOR_PIN2, LOW);
  digitalWrite(RIGHT_MOTOR_PIN1, HIGH);
  digitalWrite(RIGHT_MOTOR_PIN2, LOW);
  analogWrite(LEFT_MOTOR_PWM, leftSpeed);
  analogWrite(RIGHT_MOTOR_PWM, rightSpeed);
}