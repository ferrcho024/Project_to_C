#ifndef CONECTIVITY_H
#define CONECTIVITY_H

// Helper fuctions

/* ----- Wifi ----- */
void setup_wifi(const char *ssid, const char *password);

/* ----- MQTT ----- */
PubSubClient mqtt_connect(const char *broker_ip, int port);
void clientCallback(char* topic, byte* payload, unsigned int length); 
void reconnectMQTTClient(PubSubClient client);

#endif //CONECTIVITY_H