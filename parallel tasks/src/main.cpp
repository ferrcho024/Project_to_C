#include "functions.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

int tamanoLista;  // Tamaño de la lista que almacena los datos
int lineaInicio; // Inicializa la lectura desde la línea 0
bool ban = true;

void tarea1(void *parameter) {

  while(ban){

    tamanoLista = 60;
    lineaInicio = 0;
    
    float* miLista = leerArchivoSPIFFS("/spiffs/datos.txt", lineaInicio, &tamanoLista);
    lineaInicio = lineaInicio + tamanoLista; // Aumenta la el número de la línea para la siguiente iteracción
    //Serial.println(lineaInicio);


    if (miLista != NULL) {
          printf("Tamaño de la lista: %d\n", tamanoLista);
          printf("Lista de valores:\n");
          for (int i = 0; i < tamanoLista; i++) {
              printf("%.5f\n", miLista[i]);
              //printf("%d\n", i);
              // Obtener la fecha y hora actual
              write_data_to_file("/spiffs/data.txt", miLista[i]);
              delay(200);
          }

          // Liberar memoria después de su uso
          free(miLista);
      }

      //esp_vfs_spiffs_unregister(NULL); // Desmonta el sistema de archivo SPIFFS
      Serial.println("**** Finalizó******");
      ban = false;

  }

  ban = true;

  while(ban){


    read_data_from_file("/spiffs/data.txt");

    tamanoLista = 60;
    lineaInicio = 0;
    
    float* miLista = leerArchivoSPIFFS("/spiffs/data.txt", lineaInicio, &tamanoLista);
    lineaInicio = lineaInicio + tamanoLista; // Aumenta el número de la línea para la siguiente iteracción
    //Serial.println(lineaInicio);


    if (miLista != NULL) {
          printf("Lista de valores:\n");
          for (int i = 0; i < tamanoLista; i++) {
              printf("%.5f\n", miLista[i]);
              delay(200);
          }

          // Liberar memoria después de su uso
          free(miLista);
      }

      //esp_vfs_spiffs_unregister(NULL);
      //Serial.println("**** Finalizó******");
      ban = false;

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

  // Configurar y conectar WiFi
  ConnectToWiFi();
  
  // Inicializar SNTP
  //initialize_sntp();

  // Esperar hasta que SNTP obtenga la hora actual
  //wait_for_sntp();

  initialize_spiffs();
  crear_Archivo("/spiffs/data.txt");
  //write_data_to_file("/spiffs/data.txt", 11.11111);

  

  // Crea dos tareas y las asigna a diferentes núcleos
  xTaskCreatePinnedToCore(tarea1, "Tarea1", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(tarea2, "Tarea2", 10000, NULL, 1, NULL, 1);
}

void loop() {
    // Tu código en el bucle principal, si es necesario
}