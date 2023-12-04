#include "functions.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

int tamanoLista = 60;  // Tamaño de la lista que almacena los datos
int lineaInicio = 0; // Inicializa la lectura desde la línea 0

void tarea1(void *parameter) {
  while(true){
    
    float* miLista = leerArchivoSPIFFS("/spiffs/datos.txt", lineaInicio, tamanoLista);
    lineaInicio = lineaInicio + tamanoLista; // Aumenta la el número de la línea para la siguiente iteracción
    //Serial.println(lineaInicio);


    if (miLista != NULL) {
          printf("Lista de valores:\n");
          for (int i = 0; i < tamanoLista; i++) {
              printf("%.5f\n", miLista[i]);
              delay(500);
          }

          // Liberar memoria después de su uso
          free(miLista);
      }

      esp_vfs_spiffs_unregister(NULL);
      Serial.println("**** Finalizó******");
  }
}

void tarea2(void *parameter) {
  while (true) {
    //Serial.println("Tarea 2 ejecutándose en el núcleo 1");
    delay(2000);
  }
}

void setup() {
  Serial.begin(115200);

  // Crea dos tareas y las asigna a diferentes núcleos
  xTaskCreatePinnedToCore(tarea1, "Tarea1", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(tarea2, "Tarea2", 10000, NULL, 1, NULL, 1);
}

void loop() {
    // Tu código en el bucle principal, si es necesario
}