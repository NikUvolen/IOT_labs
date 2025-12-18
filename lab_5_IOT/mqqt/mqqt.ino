#include <GyverStepper.h>
#include "myMQQT.h"

// ================== НАСТРОЙКИ ==================
#define IN1 D5
#define IN2 D3
#define IN3 D2
#define IN4 D1
#define BTN_PIN D7

const int STEPS_PER_REV = 2048;
int motorSpeed = 700;

// ================== STEPPER ====================
GStepper<STEPPER4WIRE> stepper(STEPS_PER_REV, IN1, IN3, IN2, IN4);

// ================== СОСТОЯНИЯ ==================
enum SystemState {
  STATE_NORMAL,
  STATE_SET_OPEN_POS,
  STATE_SET_CLOSE_POS
};

SystemState currentState = STATE_NORMAL;

// ================== ПЕРЕМЕННЫЕ =================
bool isReversed = false;
bool motorOn = false;

long closeCoords = 0;       // длина хода (ВСЕГДА > 0)
long currentPos;
unsigned long lastBtnTime = 0;
unsigned long calibrationTimer = 0;
unsigned long lastSend = 0;

// ================== УТИЛИТЫ ====================
int dir() {
  return isReversed ? -1 : 1;
}

bool btnPressed() {
  if (digitalRead(BTN_PIN) == LOW && millis() - lastBtnTime > 800) {
    lastBtnTime = millis();
    return true;
  }
  return false;
}

// ================== WIFI =======================
void wifiConfig() {
  wifiConnect();
  if (!client.connected()) reconnect();
}

void setNull() {
  closeCoords = closeCoords - stepper.getCurrent();
  currentPos = 0;
  stepper.reset();
}

// ================== ДВИЖЕНИЕ ===================
void goToPercent(float percent) {
  if (closeCoords <= 0) return;

  percent = constrain(percent, 0.0f, 1.0f);

  // если реверс включен — меняем местами 0% и 100%
  if (isReversed) percent = 1.0f - percent;

  long target = lroundf(closeCoords * percent);
  stepper.setTarget(target);
}

void moveToSteps(int steps) {
  stepper.setRunMode(FOLLOW_POS);
  if (isReversed) 
    steps = -steps;
  stepper.setTarget(steps, RELATIVE);
}

// ================== КАЛИБРОВКА =================
void startSetOpenPos() {
  Serial.println("Calibration: set OPEN position");

  stepper.brake();
  stepper.reset();               // 0 = открыто
  stepper.setSpeed(motorSpeed * dir());
  stepper.setRunMode(KEEP_SPEED);

  currentState = STATE_SET_OPEN_POS;
  calibrationTimer = millis();
}

void processSetOpenPos() {
  if (btnPressed()) {
    stepper.brake();
    stepper.reset();             // жёстко фиксируем 0
    startSetClosePos();
  }

  if (millis() - calibrationTimer > 60000) {
    Serial.println("Open calibration timeout");
    stepper.brake();
    currentState = STATE_NORMAL;
  }
}

void startSetClosePos() {
  Serial.println("Calibration: set CLOSE position");

  stepper.setSpeed(-motorSpeed * dir());
  stepper.setRunMode(KEEP_SPEED);

  currentState = STATE_SET_CLOSE_POS;
  calibrationTimer = millis();
}

void processSetClosePos() {
  if (btnPressed()) {
    stepper.brake();
    closeCoords = abs(stepper.getCurrent());

    stepper.reset();
    stepper.setRunMode(FOLLOW_POS);
    currentState = STATE_NORMAL;

    Serial.printf("Calibration done. Close coords = %ld\n", closeCoords);
  }

  if (millis() - calibrationTimer > 60000) {
    Serial.println("Close calibration timeout");
    stepper.brake();
    currentState = STATE_NORMAL;
  }
}

// ================== РЕВЕРС =====================
void reverseMotor(bool rev) {
  isReversed = rev;
  stepper.brake();

  // просто переслать актуальный процент, чтобы UI обновился
  client.publish("/home/curtains/procOpen", String(getCurrentPercent()).c_str(), false);
}

// ================== ПРОЦЕНТ ====================
float getCurrentPercent() {
  if (closeCoords <= 0) return 0.0f;

  long pos = stepper.getCurrent();
  pos = constrain(pos, 0L, closeCoords);

  float percent = (float)pos / (float)closeCoords;   // 0..1

  // если реверс включен — меняем местами 0% и 100%
  if (isReversed) percent = 1.0f - percent;

  return percent * 100.0f;
}

// ================== MQTT =======================
void callback(char* topic, byte* payload, unsigned int length) {
  String data;
  for (uint8_t i = 0; i < length; i++) data += (char)payload[i];

  if (String(topic) == motor_topic + "/setNull") {
    setNull();
  }
  else if (String(topic) == motor_topic + "/reversed") {
    reverseMotor(data.toInt() == 1);
  }
  else if (String(topic) == motor_topic + "/speed") {
    motorSpeed = data.toInt();
    stepper.setMaxSpeed(motorSpeed);
  }
  else if (String(topic) == motor_topic + "/preset") {
    int p = data.toInt();
    if (p == 1) goToPercent(0.0);
    else if (p == 2) goToPercent(0.25);
    else if (p == 3) goToPercent(0.5);
    else if (p == 4) goToPercent(0.75);
    else goToPercent(1.0);
  }
  else if (String(topic) == motor_topic + "/procOpenSlider") {
    goToPercent(data.toFloat() / 100.0);
  }
  else if (String(topic) == motor_topic + "/moveSteps") {
    moveToSteps(data.toInt());
  }
  else if (String(topic) == motor_topic + "/calibration") {
    startSetOpenPos();
  }
}

// ================== SETUP ======================
void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);

  client.setServer(mqtt_server, mqtt_port);
  WiFi.mode(WIFI_STA);
  wifiConfig();
  client.setCallback(callback);

  stepper.setRunMode(FOLLOW_POS);
  stepper.setMaxSpeed(motorSpeed * dir());
  stepper.setAcceleration(600);
  stepper.autoPower(true);

  startSetOpenPos();
}

// ================== LOOP =======================
void loop() {
  wifiConnect();
  client.loop();
  stepper.tick();
  currentPos = stepper.getCurrent();

  switch (currentState) {
    case STATE_SET_OPEN_POS:
      processSetOpenPos();
      break;
    case STATE_SET_CLOSE_POS:
      processSetClosePos();
      break;
    case STATE_NORMAL:
      break;
  }

  if (closeCoords > 0 && millis() - lastSend > 5000) {
    lastSend = millis();
    float procOpen_result = getCurrentPercent();
    client.publish("/home/curtains/procOpen", String(procOpen_result).c_str(), false);
  }
}