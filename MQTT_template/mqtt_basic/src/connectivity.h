#ifndef CONNECTIVITY_H
#define CONNECTIVITY_H
#include <Wifi.h>
#include <PubSubClient.h>
#include "config.h"
#include "globals.h"

/* ----- Wifi ----- */
void setup_wifi(const char *ssid, const char *password);

/* ----- MQTT ----- */
void client_callback(char* topic, byte* payload, unsigned int length); 
void reconnect_mqtt_client(PubSubClient *client);
void add_topic(char topics[][SIZE_TOPIC], const char* topic);
#endif //CONECTIVITY_H