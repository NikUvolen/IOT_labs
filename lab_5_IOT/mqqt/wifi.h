#include <ESP8266WiFi.h>
#include "secrets.h"

char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key index number (needed only for WEP)
WiFiClient  espClient;


void wifiConnect() {
  int retryCount = 0;
  const int maxRetries = 10; // Максимальное количество попыток подключения
  
  // Проверяем текущее состояние WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Попытка подключения к сети: ");
    Serial.println(SECRET_SSID);
    
    while (WiFi.status() != WL_CONNECTED && retryCount < maxRetries) {
      Serial.print("Статус WiFi: ");
      Serial.println(WiFi.status()); // Выводим код статуса для диагностики
      
      WiFi.begin(SECRET_SSID, SECRET_PASS); // Используем константы из секретов
      
      // Ждем подключения с таймаутом 10 секунд
      unsigned long startTime = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
        delay(500);
        Serial.print(".");
      }
      
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nОшибка подключения, пробуем снова...");
        retryCount++;
        delay(2000); // Пауза перед следующей попыткой
      }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nПодключение успешно!");
      Serial.print("IP-адрес: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nНе удалось подключиться после всех попыток");
    }
  }
}