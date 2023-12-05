#include "functions.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

int tamanoLista;  // Tamaño de la lista que almacena los datos
int lineaInicio; // Inicializa la lectura desde la línea 0
bool ban = true;
int frec = 1000; // Espacio de tiempo entre los datos que llegan (en milisegundos)

void tarea1(void *parameter) {

  lineaInicio = 0;
  while(ban){

    tamanoLista = 60;
    
    float* miLista = leerArchivoSPIFFS("/spiffs/datos.txt", lineaInicio, &tamanoLista);
    lineaInicio = lineaInicio + tamanoLista; // Aumenta la el número de la línea para la siguiente iteracción
    //Serial.println(lineaInicio);


    if (miLista != NULL) {
          printf("\nLista de valores:\n");
          printf("Tamaño de la lista: %d\n", tamanoLista);
          for (int i = 0; i < tamanoLista; i++) {
              printf("%.5f\n", miLista[i]);
              //printf("%d\n", i);
              // Obtener la fecha y hora actual
              write_data_to_file("/spiffs/data.txt", miLista[i]);
              delay(frec);
          }

          // Liberar memoria después de su uso
          free(miLista);
      }

      //esp_vfs_spiffs_unregister(NULL); // Desmonta el sistema de archivo SPIFFS
      Serial.println("**** Finalizó******");
      ban = false;

      delay(frec);

      ban = true;

  }

  //ban = true;

  while(ban){

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
    //printf("Tarea 2 ejecutándose en el núcleo 1 %d\n", ban);
    delay(frec);
    if (!ban){
      float* datos = leerArchivoSPIFFS("/spiffs/temp.txt", 0, &tamanoLista); // Lee el archivo solo con valores
      float p_com = completitud(datos, tamanoLista);
      printf("**********Porcentaje completitud: %.5f\n", p_com);
      //read_data_from_file("/spiffs/data.txt"); // Lee el archivo con formato de hora y valor float
      crear_Archivo("/spiffs/temp.txt");
      //Serial.println("Tarea 2 ejecutándose en el núcleo 1");
      delay(frec);
    }
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
  crear_Archivo("/spiffs/temp.txt"); // Archivo de memoria temporal
  crear_Archivo("/spiffs/data.txt"); // Archivo de memoria permanente
  //write_data_to_file("/spiffs/data.txt", 11.11111);

  

  // Crea dos tareas y las asigna a diferentes núcleos
  xTaskCreatePinnedToCore(tarea1, "Tarea1", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(tarea2, "Tarea2", 10000, NULL, 1, NULL, 1);
}

void loop() {
    // Tu código en el bucle principal, si es necesario
}