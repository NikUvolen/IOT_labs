#include <PubSubClient.h>
#include "wifi.h"

// config --- MQTT ----
const char* mqtt_server = "m1.wqtt.ru";
const int mqtt_port = 20114;
const char* mqtt_user = "u_QU4PSK";
const char* mqtt_password = "TfEMphQO";

const String motor_topic = "/home/curtains";

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
