#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "builder.h"

#define cmdOpenPin D5
#define cmdClosePin D6
#define STOP_TIMEOUT 10000 

void setup() {
  Serial.begin(115200);
  pinMode(MOTOR_PIN1, OUTPUT);
  pinMode(MOTOR_PIN2, OUTPUT);
  pinMode(cmdOpenPin, INPUT_PULLUP);
  pinMode(cmdClosePin, INPUT_PULLUP);

  LittleFS.begin();
  db.begin();
  db.init(systemVars::speed, 255);
  motorSpeed = db.get(systemVars::speed).toInt();

  WiFi.mode(WIFI_AP);
  WiFi.softAP("myESP", "qwerty0000");

  Serial.print("\nip: ");
  Serial.println(WiFi.softAPIP());

  sett.begin();
  sett.onBuild(build);
  sett.onUpdate(update);
}

void loop() {
  sett.tick();
  motorSpeed = db.get(systemVars::speed).toInt();

  // Отладочная информация
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    Serial.print("State: ");
    Serial.print(motorState);
    Serial.print(" | CmdOpen: ");
    Serial.print(commandOpen ? "ON" : "OFF");
    Serial.print(" | CmdClose: ");
    Serial.print(commandClose ? "ON" : "OFF");
    Serial.print(" | OpenPin: ");
    Serial.print(digitalRead(cmdOpenPin));
    Serial.print(" | ClosePin: ");
    Serial.println(digitalRead(cmdClosePin));
    lastPrint = millis();
  }

  // Проверяем концевики в первую очередь
  bool openLimitReached = (digitalRead(cmdOpenPin) == LOW);
  bool closeLimitReached = (digitalRead(cmdClosePin) == LOW);
  
  // Если достигли концевика, останавливаемся независимо от команд
  if (openLimitReached && (motorState == MOTOR_OPENING || commandOpen)) {
    stop();
    motorState = MOTOR_OPEN;
    commandOpen = false;
    commandClose = false; // Сбрасываем все команды
    motorStartTime = 0;
    Serial.println("Open limit reached");
  }
  
  if (closeLimitReached && (motorState == MOTOR_CLOSING || commandClose)) {
    stop();
    motorState = MOTOR_CLOSED;
    commandOpen = false;
    commandClose = false; // Сбрасываем все команды
    motorStartTime = 0;
    Serial.println("Close limit reached");
  }

  if (commandOpen) {
    // Проверяем не нажаты ли концевики
    if (openLimitReached) {
      // Уже достигнут концевик открытия
      stop();
      motorState = MOTOR_OPEN;
      commandOpen = false;
      motorStartTime = 0;
    } else {
      // Двигаемся к открытию
      open();
      motorState = MOTOR_OPENING;
      
      if (motorStartTime == 0) {
        motorStartTime = millis();
      }
      
      // Проверка таймаута
      if (millis() - motorStartTime > STOP_TIMEOUT) {
        stop();
        motorState = MOTOR_STOPPED;
        commandOpen = false;
        motorStartTime = 0;
        Serial.println("Timeout - stopped opening!");
      }
    }
  } 
  // Обработка команд закрытия
  else if (commandClose) {
    // Проверяем не нажаты ли концевики
    if (closeLimitReached) {
      // Уже достигнут концевик закрытия
      stop();
      motorState = MOTOR_CLOSED;
      commandClose = false;
      motorStartTime = 0;
    } else {
      // Двигаемся к закрытию
      close();
      motorState = MOTOR_CLOSING;
      
      if (motorStartTime == 0) {
        motorStartTime = millis();
      }
      
      // Проверка таймаута
      if (millis() - motorStartTime > STOP_TIMEOUT) {
        stop();
        motorState = MOTOR_STOPPED;
        commandClose = false;
        motorStartTime = 0;
        Serial.println("Timeout - stopped closing!");
      }
    }
  }
  // Нет активных команд
  else if (motorState == MOTOR_OPENING || motorState == MOTOR_CLOSING) {
    stop();
    motorState = MOTOR_STOPPED;
    motorStartTime = 0;
  }
}