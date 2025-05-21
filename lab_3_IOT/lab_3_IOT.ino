#include "ThingSpeak.h"
#include "secrets.h"
#include "DHT11.h"
#include <OneWire.h>
#include <DallasTemperature.h>

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

#include <ESP8266WiFi.h>

char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key index number (needed only for WEP)
WiFiClient  client;

unsigned long lastPrintTime = 0;
unsigned long lastSendTime = 0;
const unsigned long printInterval = 1000;
const unsigned long sendInterval = 15000;
unsigned long currentTime;

// init dht11 pin
#define DHT_PIN D4
// init ky-001 pin
#define TEMP_PIN D3
// init hc-sr04 pin
#define TRIGGER_PIN D6
#define ECHO_PIN D7
#define MAX_DISTANCE 400

const int numValues = 15;
// Config DHT
DHT11 dht11(DHT_PIN);
int dhtValues[numValues] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
unsigned int dhtCurrentValue = 0;
// Config KY-001
OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);
float maxTemp = -3.4028235E+38;
float minTemp = 3.4028235E+38;

void setup() {
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIGGER_PIN, LOW);
  Serial.begin(115200);

  sensors.begin();
  Serial.println("KY-001 (DS18B20) Temperature Sensor Test");

  delay(100);

  WiFi.mode(WIFI_STA);

  ThingSpeak.begin(client);
}

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

int readHumi() {
  int humidity;
  do {
    // Убираем 100-ки
    humidity = dht11.readHumidity();
    if (humidity >= 100) {
      Serial.println("DHT11 100 ERROR");
      humidity = 100;
    }
  } while (humidity == 100);

  dhtValues[dhtCurrentValue] = humidity;
  dhtCurrentValue++;
  if (dhtCurrentValue >= 15)
    dhtCurrentValue = 0;

  return humidity;
}

float calcAvgHumi() {
  float sumValue = 0;
  int numValue = 0;
  for (int i = 0; i < numValues; i++) {
    if (dhtValues[i] != -1) {
      sumValue += dhtValues[i];
      numValue++;
    }
  }
  float result = (sumValue * 1.0) / numValue;
  return result;
}

long getEchoTiming() {
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration;
}

long getDistance() {
  long distanceCM = getEchoTiming() * 1.7 * .01;
  return distanceCM;
}

float getTemp() {
  sensors.requestTemperatures(); 
  // Получение температуры в градусах Цельсия
  float tempC = sensors.getTempCByIndex(0);
  return tempC;
}

void getMinAndMaxTemp(float tempC) {
  if (tempC > maxTemp)
    maxTemp = tempC;
  if (tempC < minTemp)
    minTemp = tempC;
}

void printTemp(float tempC) {
  // Проверка на ошибку чтения (может вернуть -127 при проблемах)
  if (tempC != DEVICE_DISCONNECTED_C) {
    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.println(" °C");
  } else {
    Serial.println("Error: Could not read temperature data!");
  }
}

void sendData(int humi, long dhtData, float tempData, float avgHumi, float maxTempC, float minTempC) {
  ThingSpeak.setField(1, humi);
  ThingSpeak.setField(2, dhtData);
  ThingSpeak.setField(3, tempData);
  ThingSpeak.setField(4, avgHumi);
  ThingSpeak.setField(5, maxTempC);
  ThingSpeak.setField(6, minTempC);

  int httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if (httpCode == 200) {
    Serial.printf("\n----------\nChannel write successful. DATA SEND\n----------\n\n");
  }
  else {
    Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
  }
}

void loop() {
  wifiConnect();
  currentTime = millis();

  int humidity = readHumi(); 
  float avgHumi = calcAvgHumi();
  long distance = getDistance();
  float temp = getTemp();

  if (currentTime - lastPrintTime >= printInterval) {
    // print data
    lastPrintTime = millis();
    Serial.printf("------------DATA-----------\n\n");
    Serial.printf("Humi = %d\n", humidity);
    Serial.printf("AVG Humi = %f\n", avgHumi);
    Serial.printf("Distance = %d cm\n", distance);
    Serial.printf("Min temp: %f\nMax temp: %f\n", minTemp, maxTemp);
    printTemp(temp);
    Serial.printf("---------------------------\n\n");
    getMinAndMaxTemp(temp);
  }
  if (currentTime - lastSendTime >= sendInterval) {
    // send data
    lastSendTime = millis();
    sendData(humidity, distance, temp, avgHumi, maxTemp, minTemp);
  }
}
