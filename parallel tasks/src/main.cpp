#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "parameters.h"
#include "file_func.h"
#include "dimensions.h"

int listSize;  // Tamaño de la lista que almacena los values
int startline; // Inicializa la lectura desde la línea 0
bool ban = true;
int frec = 1000; // Espacio de tiempo entre los values que llegan (en milisegundos)

void task1(void *parameter) {

  startline = 0;
  while(ban){

    listSize = 60;
    
    float* myList = read_file(df, startline, &listSize);
    startline = startline + listSize; // Aumenta la el número de la línea para la siguiente iteracción
    //Serial.println(startline);


    if (myList != NULL) {
          printf("\nList of values:\n");
          printf("List size: %d\n", listSize);
          for (int i = 0; i < listSize; i++) {
              printf("%.5f\n", myList[i]);
              //printf("%d\n", i);
              // Obtener la fecha y hora actual
              write_data_to_file(data, myList[i]);
              delay(frec);
          }

          // Liberar memoria después de su uso
          free(myList);
      }

      //esp_vfs_spiffs_unregister(NULL); // Desmonta el sistema de archivo SPIFFS
      Serial.println("**** Finished ******");
      ban = false;

      delay(frec);

      ban = true;

  }

}

void task2(void *parameter) {
  while (true) {
    //printf("Tarea 2 ejecutándose en el núcleo 1 %d\n", ban);
    delay(frec);
    if (!ban){
      float* values = read_file(temp, 0, &listSize); // Lee el archivo solo con valores
      float p_com = completeness(values, listSize);
      printf("********** Completeness: %.5f\n", p_com);
      //read_data_from_file("/spiffs/data.txt"); // Lee el archivo con formato de hora y valor float
      create_file(temp);
      //Serial.println("Tarea 2 ejecutándose en el núcleo 1");
      delay(frec);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Configurar y conectar WiFi
  ConnectToWiFi();
  
  initialize_spiffs();
  create_file(temp); // Archivo de memoria temporal
  create_file(data); // Archivo de memoria permanente
  //write_data_to_file("/spiffs/data.txt", 11.11111);

  

  // Crea dos tareas y las asigna a diferentes núcleos
  xTaskCreatePinnedToCore(task1, "Task1", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(task2, "Task2", 10000, NULL, 1, NULL, 1);
}

void loop() {
    // Tu código en el bucle principal, si es necesario
}