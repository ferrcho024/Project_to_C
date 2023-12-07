#include <Wifi.h>
#include <PubSubClient.h>
#include "conectivity.h"

extern const char *ID; 
extern const char *BROKER; 
extern const char *topic1; 

// Wifi client
// WiFiClient wifiClient;

// MQTT client
// PubSubClient mqttClient(wifiClient);  

// Wifi conection
void setup_wifi(const char *ssid, const char *password) {
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password); // Connect to network

  while (WiFi.status() != WL_CONNECTED) { // Wait for connection
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}


// MQTT conectiobn
PubSubClient mqtt_connect(const char *broker_ip, int port) {
  PubSubClient client;
  client.setServer(broker_ip, port);
  client.setCallback(clientCallback);
  return client;
}

void reconnectMQTTClient(PubSubClient client) {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect(ID)) {
      Serial.print("connected to Broker: ");
      Serial.println(BROKER);
      // Topic(s) subscription
      client.subscribe(topic1);
    }
    else {
      Serial.print("Retying in 5 seconds - failed, rc=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}


// Handle incomming messages from the broker
void clientCallback(char* topic, byte* payload, unsigned int length) {
  //String response;
  Serial.println("*****");

  /*

  for (int i = 0; i < length; i++) {
    response += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(TOPIC.c_str());
  Serial.print("] ");
  Serial.println(response);
  if(response == "ON")  // Turn the light on
  {
    digitalWrite(LIGHT_PIN, HIGH);
  }
  else if(response == "OFF")  // Turn the light off
  {
    digitalWrite(LIGHT_PIN, LOW);
  }
  */
}