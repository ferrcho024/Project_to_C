/*  Temperature Sensor and light controller
 *  (Light is represented using the built-in led)
 *  IoT Course 
 *  Universidad de Antioquia
 */


#include <Wifi.h>
#include <PubSubClient.h>
#include "config.h"

/* ----- Puertos ----- */
// Definicion de los pines
//...


/* ----- Wifi ----- */
//const char* ssid = "SSID-AP";   // name of your WiFi network
//const char* password = "PASS_AP"; // password of the WiFi network
WiFiClient wifiClient;


#include <OneWire.h>
#include <DallasTemperature.h>

//Selecciona el pin al que se conecta el sensor de temperatura
const int oneWireBus = 4;


#define BUILTIN_LED 2
int light_state = 0; // Initial led's state

//Comunicar que vamos a utilizar la interfaz oneWire
OneWire oneWire(oneWireBus);

//Indica que el sensor utilizará la interfaz OneWire
DallasTemperature sensors (&oneWire);

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);    // BUILTIN_LED pin as an output
  digitalWrite(BUILTIN_LED, LOW); // Light initial state is OFF

  //Ajustar la velocidad para el monitor serie
  Serial.begin(115200);
  sensors.begin();
  SPIFFS.begin();
  ConnectWiFi_STA(false);
  InitMqtt();

}


char msg[50];

void loop() {

  //Leer la temperatura
  Serial.print("Mandando comandos a los sensores ");

  sensors.requestTemperatures();

  //Lectura en grados celsius
  float temperatureC = sensors.getTempCByIndex(0);

  //Escribir los datos en el monitor de serie
  Serial.print("Temperatura sensor : ");
  Serial.print(temperatureC);
  Serial.println("°C");

  HandleMqtt();

  snprintf (msg, 75, "%.2f", temperatureC);
  Serial.print("Publish message: ");
  Serial.println(msg);
  PublisMqttString("casa/tmp", msg);

  // Lectura de la temperatura cada 5 segundos
  delay(5000);
}