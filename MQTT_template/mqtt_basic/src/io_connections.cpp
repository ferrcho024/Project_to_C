#include <Arduino.h>
#include "io_connections.h"

void setup_ports(void) {
    led_pin = LED_PIN; // GPIO2
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, LOW);
}