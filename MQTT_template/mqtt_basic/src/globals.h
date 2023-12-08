#ifndef GLOBALS_H
#define GLOBALS_H
#include "config.h"

#ifdef LOCAL
#define EXTERN
#else
#define EXTERN extern
#endif

#define SIZE 100
#define MAX_TOPICS 10
#define SIZE_TOPIC 200


/* ----- PORTS CONFIGURATION  ----- */
EXTERN int led_pin;

/* ----- SERIAL PORT ----- */
EXTERN int serial_speed;

/* ----- Wifi ----- */
EXTERN char ssid[SIZE];           // name of your WiFi network
EXTERN char password[SIZE];   // password of the WiFi network

/* ----- MQTT Broker ----- */
EXTERN char broker[SIZE]; // IP address of your MQTT 
EXTERN char id[SIZE];  // Name of our device, must be unique

/* ----- MQTT topics----- */
EXTERN char topics[MAX_TOPICS][SIZE_TOPIC];
EXTERN int num_topics;

#endif //CONFIG_H