#include <GyverStepper.h>
#include "myMQQT.h"

const int stepsPerRevolution = 2048;
#define IN1 D5
#define IN2 D3
#define IN3 D2
#define IN4 D1
#define BTN_PIN D7

GStepper<STEPPER4WIRE> stepper(2048, IN1, IN3, IN2, IN4);

// Состояния системы
enum SystemState {
  STATE_NORMAL,
  STATE_SET_OPEN_POS,
  STATE_SET_CLOSE_COORDS
};

SystemState currentState = STATE_NORMAL;
bool motorOn = false;
int motorSpeed = 700;
long closeCoords = 0;
long currentPos, lastSend;
unsigned long lastPressBTN = 0;
unsigned long calibrationStartTime = 0;

void wifiConfig() {
  wifiConnect();
  if (!client.connected()) {
    reconnect();
  }
}

bool getBtnPress() {
  if ((digitalRead(BTN_PIN) == HIGH) && (millis() - lastPressBTN > 1000)) {
    lastPressBTN = millis();
    return true;
  }  
  return false;
}

void changeMotorSpeed(int speed) {
  motorSpeed = speed;
  stepper.setSpeed(motorSpeed);  // Установка текущей скорости
  stepper.setMaxSpeed(motorSpeed); // Установка максимальной скорости
}

void goTo(float proc) {
  if (closeCoords == 0) return;
  long goToCoord = floor(closeCoords * proc);
  stepper.setTarget(goToCoord);
}

void startSetOpenPos() {
  currentState = STATE_SET_OPEN_POS;

  stepper.setRunMode(KEEP_SPEED);
  stepper.setSpeed(-motorSpeed);
  calibrationStartTime = millis();
  Serial.println("Start setting open position");
}

void processSetOpenPos() {
  if (getBtnPress()) {
    stepper.reset();
    Serial.println("Open position set");
    startSetCloseCoords();
  }
  
  // freeze defend (60 sec timeout)
  if (millis() - calibrationStartTime > 60000) {
    currentState = STATE_NORMAL;
    stepper.brake();
    Serial.println("Open position timeout");
  }
}

void startSetCloseCoords() {
  currentState = STATE_SET_CLOSE_COORDS;

  stepper.setRunMode(KEEP_SPEED);
  stepper.setSpeed(motorSpeed);
  calibrationStartTime = millis();
  Serial.println("Start setting close position");
}

void processSetCloseCoords() {
  if (getBtnPress()) {
    stepper.brake();
    closeCoords = stepper.getCurrent();
    stepper.setRunMode(FOLLOW_POS);
    currentState = STATE_NORMAL;
    Serial.printf("Close position set: %d\n", closeCoords);
  }

  // freeze defend (60 sec timeout)
  if (millis() - calibrationStartTime > 60000) {
    currentState = STATE_NORMAL;
    stepper.brake();
    Serial.println("Close position timeout");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String data_pay;
  for (int i = 0; i < length; i++) {
    data_pay += String((char)payload[i]);
  }
    
  if (String(topic) == (motor_topic + "/on")) {
    motorOn = (data_pay == "ON" || data_pay == "1") ? true : false;
    Serial.printf("On/Off: %s\n", data_pay);
  }
  else if (String(topic) == (motor_topic + "/speed")) {
    Serial.printf("speed: %s\n", data_pay);
    changeMotorSpeed(data_pay.toInt());
  }
  else if (String(topic) == (motor_topic + "/direction")) {
    Serial.printf("Direction: %s\n", data_pay);
    if (data_pay == "1") goTo(0.0);
    else if (data_pay == "2") goTo(0.25);
    else if (data_pay == "3") goTo(0.5);
    else if (data_pay == "4") goTo(0.75);
    else goTo(1.0);
  }
  else if (String(topic) == (motor_topic + "/procOpenSlider")) {
    Serial.printf("Direction: %s\n", data_pay);
    float newPos = data_pay.toFloat() / 100;
    Serial.println(newPos);
    goTo(newPos);
  }
  else if (String(topic) == (motor_topic + "/сolibration")) {
    startSetOpenPos();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT); 

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); 

  WiFi.mode(WIFI_STA);
  wifiConfig();

  lastSend = millis();

  stepper.setRunMode(FOLLOW_POS);
  stepper.setMaxSpeed(motorSpeed);
  stepper.setAcceleration(600);
  
  // Первоначальная калибровка
  startSetOpenPos();
}

void loop() {
  wifiConnect();
  client.loop();
  
  stepper.tick();

  switch (currentState) {
    case STATE_SET_OPEN_POS:
      processSetOpenPos();
      break;
    case STATE_SET_CLOSE_COORDS:
      processSetCloseCoords();
      break;
    case STATE_NORMAL:
      // Нормальная работа
      break;
  }
  
  if (stepper.getCurrent() != currentState && millis() - lastSend > 5000) {
    lastSend = millis();
    currentPos = stepper.getCurrent();
    client.publish("/home/curtains/procOpen", String(currentPos).c_str(), false);
  }
  
  delay(1);
}