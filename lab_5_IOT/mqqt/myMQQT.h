#include <PubSubClient.h>
#include "wifi.h"

// config --- MQTT ----
const char* mqtt_server = "m8.wqtt.ru";
const int mqtt_port = 20180;
const char* mqtt_user = "u_8O9BTW";
const char* mqtt_password = "dNlCq2E7";

const String motor_topic = "/home/сurtains";

PubSubClient client(espClient);
// --------------------


void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266-" + WiFi.macAddress();
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password) ) {
      Serial.println("connected");
      
      client.subscribe( (motor_topic + "/#").c_str() );

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
