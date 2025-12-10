#include <GyverDBFile.h>
#include <LittleFS.h>
#include <SettingsGyver.h>

#include "motorFuncs.h"

GyverDBFile db(&LittleFS, "/data.db");
SettingsGyver sett("myDB", &db); 

unsigned long motorStartTime = 0;
bool manualStop = false;

void build(sets::Builder& b) {
  if (b.beginGroup("DoorControl")) {
    // Упрощаем логику кнопок - они должны быть всегда доступны
    if (b.Button("Open")) {
      // Разрешаем открытие если не закрыто и не закрывается
      if (motorState != MOTOR_OPEN) {
        commandOpen = true;
        commandClose = false;
      }
    }
    if (b.Button("Close")) {
      // Разрешаем закрытие если не открыто и не открывается
      if (motorState != MOTOR_CLOSED) {
        commandClose = true;
        commandOpen = false;
      }
    }
    // Кнопка Stop для принудительной остановки
    if (b.Button("Stop")) {
      commandOpen = false;
      commandClose = false;
      motorState = MOTOR_STOPPED;
      motorStartTime = 0; // Важно сбросить таймер
    }
    
    b.Slider(systemVars::speed, "Speed", 0, 255);
    
    const char* motorStateStr = "";
    switch (motorState) {
      case MOTOR_OPEN:
        motorStateStr = "Open";
        break;
      case MOTOR_CLOSED:
        motorStateStr = "Closed";
        break;
      case MOTOR_OPENING:
        motorStateStr = "Opening";
        break;
      case MOTOR_CLOSING:
        motorStateStr = "Closing";
        break;
      case MOTOR_STOPPED:
        motorStateStr = "Stopped";
        break;
      default:
        motorStateStr = "Unknown";
        break;
    }
    b.Label(0, "State", motorStateStr);
    b.endGroup();
  }
}

void update(sets::Updater& upd) {
  upd.update(systemVars::speed, motorSpeed);
}