#ifndef CONFIG_H
#define CONFIG_H

/* ----- Wifi ----- */
// const char* ssid = "SSID-AP";   // name of your WiFi network
// const char* password = "PASS_AP"; // password of the WiFi network
const char* ssid = "Alberto"; 
const char* password = "22181224";


/* ----- MQTT ----- */
const char *BROKER = "192.168.1.35"; // IP address of your MQTT 
const char *ID = "id_thing";  // Name of our device, must be unique

/* ----- SERIAL PORT ----- */
//const int PORT_SPEED = 9600;

// Topics
const char *topic1 = "test"; 
//const char *topic2 = "topic2_name";  

#endif //CONFIG_H


