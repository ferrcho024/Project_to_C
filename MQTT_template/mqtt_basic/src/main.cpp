#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include "conectivity.h"
#include "io_connections.h"

extern const int LED_PIN;
const int PORT_SPEED = 9600;
//extern const char* ssid;
//extern const char* password; 

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);  


void setup() {
  // Setup ports
  setup_ports();
  // Serial setup
  Serial.begin(PORT_SPEED);
  Serial.println("Serial port OK");
  // Setup wifi
  setup_wifi(ssid, password);
  // MQTT setup
  mqttClient = mqtt_connect(BROKER, 1883);
  reconnectMQTTClient(mqttClient);
}

void loop() {
  // put your main code here, to run repeatedly:
  reconnectMQTTClient(mqttClient);
  mqttClient.loop();
  delay(500);
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
}
