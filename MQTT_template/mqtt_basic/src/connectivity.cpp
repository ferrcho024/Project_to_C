#include <Wifi.h>
#include <PubSubClient.h>
#include "connectivity.h"


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

void reconnect_mqtt_client(PubSubClient *client) {
  while (!client->connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client->connect(id)) {
      Serial.print("connected to Broker: ");
      Serial.println(broker);
      // Topic(s) subscription
      client->subscribe(topics[0]);
      client->subscribe(topics[1]);
      // ...
    }
    else {
      Serial.print("Retying in 5 seconds - failed, rc=");
      Serial.println(client->state());
      delay(5000);
    }
  }
}

void add_topic(char topics[][SIZE_TOPIC], const char* topic) {
  /*
  TODO:
  - Evitar que se agreguen mas topics del numero permitido
  */
  // Add new topic to subs topics
  // strcpy(topics[num_topics++], topic)
  strcpy(*(topics + num_topics), topic); 
  num_topics++;
}

// Handle incomming messages from the broker
void client_callback(char* topic, byte* payload, unsigned int length) {
  String response;
  for (int i = 0; i < length; i++) {
    response += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(response);
  // Add the logic code
  // ....
}