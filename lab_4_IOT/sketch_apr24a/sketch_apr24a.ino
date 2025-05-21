#include <Arduino.h>
#include <GyverDBFile.h>
#include <LittleFS.h>
#include <SettingsGyver.h>
#include <ESP8266WiFi.h>
#include "GyverMotor.h"

// Конфигурация выводов
#define DRIVER_PIN1 D1  // Вывод 1 драйвера
#define DRIVER_PIN2 D2  // Вывод 2 драйвера
#define SENSOR_UP D5    // Датчик верхнего положения
#define SENSOR_DOWN D6  // Датчик нижнего положения

// Инициализация двигателя
GMotor curtainMotor(DRIVER2WIRE, DRIVER_PIN1, DRIVER_PIN2, HIGH);

// База данных и настройки
GyverDBFile storage(&LittleFS, "/config.db");
SettingsGyver webPanel("Smart Curtain", &storage);

enum ConfigIDs : size_t {
  DIRECTION_FLAG,
  MOTOR_POWER
};

bool movingUp = true;  // Направление движения
int powerLevel = 60;  // Мощность двигателя (0-255)

// void createWebInterface(sets::Builder& ui) {
//     if (ui.beginGroup("Управление")) {
//         // Регулятор мощности
//         ui.Slider(ConfigIDs::MOTOR_POWER, "Сила движения", 50, 255, 5, " ", powerLevel);
        
//         // Кнопка управления
//         if (ui.Button(movingUp ? "Опустить штору" : "Поднять штору")) {
//             if (movingUp) {
//                 // Движение вниз
//                 while (digitalRead(SENSOR_DOWN) == HIGH) {
//                     curtainMotor.setMode(AUTO);
//                     curtainMotor.setSpeed(-powerLevel);
//                     Serial.println("Движение вниз");
//                     delay(15);
//                 }
//                 curtainMotor.setMode(STOP);
//                 Serial.println("Нижнее положение");
//             } else {
//                 // Движение вверх
//                 while (digitalRead(SENSOR_UP) == HIGH) {
//                     curtainMotor.setMode(AUTO);
//                     curtainMotor.setSpeed(powerLevel);
//                     Serial.println("Движение вверх");
//                     delay(15);
//                 }
//                 curtainMotor.setMode(STOP);
//                 Serial.println("Верхнее положение");
//             }
//             movingUp = !movingUp;
//             storage.set(ConfigIDs::DIRECTION_FLAG, movingUp);
//             storage.update();
//         }
        
//         // Индикатор направления
//         ui.Switch(ConfigIDs::DIRECTION_FLAG, "Текущее направление");
//         ui.endGroup();
//     }
// }

void syncSettings(sets::Updater& sync) {
    sync.update(ConfigIDs::DIRECTION_FLAG, movingUp);
    sync.update(ConfigIDs::MOTOR_POWER, powerLevel);
}

void initializeSystem() {
    Serial.begin(115200);
    
    // Инициализация файловой системы
    if (!LittleFS.begin()) {
        Serial.println("Ошибка инициализации памяти");
    } else {
        Serial.println("Файловая система готова");
    }

    // Настройка датчиков
    pinMode(SENSOR_UP, INPUT_PULLUP);
    pinMode(SENSOR_DOWN, INPUT_PULLUP);
    
    // Настройка WiFi
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Smart-Curtain", "home1234");
    Serial.print("Адрес панели: ");
    Serial.println(WiFi.softAPIP());
    
    // Инициализация хранилища
    storage.begin();
    
    // Запуск веб-интерфейса
    // webPanel.begin();
    // webPanel.onBuild(createWebInterface);
    // webPanel.onUpdate(syncSettings);
}

void mainLoop() {
    // Обслуживание веб-интерфейса
    webPanel.tick();
}