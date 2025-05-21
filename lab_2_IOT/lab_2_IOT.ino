/*
   -- New project --
   
   This source code of graphical user interface 
   has been generated automatically by RemoteXY editor.
   To compile this code using RemoteXY library 3.1.13 or later version 
   download by link http://remotexy.com/en/library/
   To connect using RemoteXY mobile app by link http://remotexy.com/en/download/                   
     - for ANDROID 4.15.01 or later version;
     - for iOS 1.12.1 or later version;
    
   This source code is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.    
*/

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// можете включить вывод отладочной информации в Serial на 115200
//#define REMOTEXY__DEBUGLOG    

// определение режима соединения и подключение библиотеки RemoteXY 
#define REMOTEXY_MODE__WIFI_CLOUD

#include <ESP8266WiFi.h>

// настройки соединения 
#define REMOTEXY_WIFI_SSID "OnePlus 9RT 5G"
#define REMOTEXY_WIFI_PASSWORD "qwerty0000"
#define REMOTEXY_CLOUD_SERVER "cloud.remotexy.com"
#define REMOTEXY_CLOUD_PORT 6376
#define REMOTEXY_CLOUD_TOKEN "f2da2e2762b8ebfda09374dec298bb63"


#include <RemoteXY.h>

// конфигурация интерфейса RemoteXY  
#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 63 bytes
  { 255,9,0,0,0,56,0,19,0,0,0,0,30,1,106,200,1,1,4,0,
  2,22,12,63,29,0,2,26,31,31,79,78,0,79,70,70,0,6,4,104,
  71,71,17,8,4,80,113,14,55,0,2,26,7,19,60,67,18,110,64,2,
  29,2,3 };
  
// структура определяет все переменные и события вашего интерфейса управления 
struct {

    // input variables
  uint8_t switch_01; // =1 если переключатель включен и =0 если отключен
  uint8_t rgb_01_r; // =0..255 значение Красного цвета
  uint8_t rgb_01_g; // =0..255 значение Зеленого цвета
  uint8_t rgb_01_b; // =0..255 значение Синего цвета
  int8_t slider_01; // oт 0 до 100
  float edit_01;

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)
 
 
/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

#define PIN_RED D2
#define PIN_GREEN D3
#define PIN_BLUE D4
#define arrSize 3
const uint8_t pins[arrSize] = {
  PIN_RED, PIN_GREEN, PIN_BLUE
};


void setup() 
{
  RemoteXY_Init (); 
  
  for (int i = 0; i < arrSize; i++)
    pinMode(pins[i], OUTPUT);
}

void offPins() {
  for (int i = 0; i < arrSize; i++)
    analogWrite(pins[i], 0);
}

void onPins() {
  float brightness = RemoteXY.slider_01 * 0.01;
  analogWrite(PIN_RED, RemoteXY.rgb_01_r * brightness);
  analogWrite(PIN_GREEN, RemoteXY.rgb_01_g * brightness);
  analogWrite(PIN_BLUE, RemoteXY.rgb_01_b * brightness);
}

void loop() 
{ 
  RemoteXY_Handler();
  
  float time = RemoteXY.edit_01 * 1000;
  if (time < 0)
    time = time * -1;

  if (RemoteXY.switch_01 == 1) {  // Светодиод включен
    onPins();
    if (time) {
      RemoteXY_delay(time);
      offPins();
      RemoteXY_delay(time);
    }
  }
  else {  // Светодиод выключен
    offPins();
  }
}