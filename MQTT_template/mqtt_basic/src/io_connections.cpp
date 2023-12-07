#include <Arduino.h>
#include "io_connections.h"

void setup_ports(void) {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
}