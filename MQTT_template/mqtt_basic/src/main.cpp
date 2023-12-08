#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"

#define LOCAL
#include "globals.h"
#include "io_connections.h"
#include "connectivity.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);  

void setup() {
  // Init Serial port
  serial_speed = SERIAL_SPEED; // Serial Speed
  Serial.begin(serial_speed);
  // Init global parameters
  Serial.println("Inicializacion de parametros globales");
  strcpy(ssid, SSID);          // SSID
  strcpy(password, PASSWORD);  // SSID password
  strcpy(broker, BROKER);      // Broker
  strcpy(id, ID);              // ID thing
  // Setup ports
  setup_ports();
  // add subscriber topics
  Serial.println("Topicos suscritos");
  add_topic(topics, TOPIC1);
  add_topic(topics, TOPIC2);
  //Serial.println(TOPIC1);
  //Serial.println(*topics[1]);
  Serial.println(topics[0]);
  Serial.println(topics[1]);
  
  //Serial.println("Serial port OK");
  //Setup wifi
  setup_wifi(ssid, password);
  // MQTT setup
  mqttClient.setServer(broker, MQTT_PORT);
  mqttClient.setCallback(client_callback);
  reconnect_mqtt_client(&mqttClient);
}

void loop() {
  // put your main code here, to run repeatedly:
  reconnect_mqtt_client(&mqttClient);
  mqttClient.loop();
  delay(500);
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
}

