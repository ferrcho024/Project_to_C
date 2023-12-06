#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "parameters.h"
#include "file_func.h"
#include "dimensions.h"

int listSize;  // Tamaño de la lista que almacena los values
int startline; // Inicializa la lectura desde la línea 0
int siataValue; // Contador para extraer el valor de la estación SIATA
bool ban = true;
int frec = 1000; // Espacio de tiempo entre los values que llegan (en milisegundos)

void task1(void *parameter) {

  startline = 0;
  siataValue = -1;
  while(true){
    delay(frec/2);
    if (ban){

      listSize = 60;
      
      float* myList_df = read_file(df, startline, &listSize);
      float* myList_nova = read_file(nova, startline, &listSize);
      startline = startline + listSize; // Aumenta la el número de la línea para la siguiente iteracción
      //Serial.println(startline);


      if (myList_df != NULL) {
            printf("\nList of values:\n");
            printf("List size: %d\n", listSize);
            for (int i = 0; i < listSize; i++) {
                //printf("%.5f\n", myList[i]);
                //printf("%d\n", i);
                // Obtener la fecha y hora actual
                write_data_to_file(data, myList_df[i], myList_nova[i]);
                //delay(frec);
            }

            // Liberar memoria después de su uso
            free(myList_df);
            free(myList_nova);
        }

        //esp_vfs_spiffs_unregister(NULL); // Desmonta el sistema de archivo SPIFFS
        Serial.println("**** Finished ******");
        ban = false;
        siataValue += 1;

        //delay(frec);

        //ban = true;

    }
  }

}

void task2(void *parameter) {
  float dimen[24][8];
  while (true) {
    //printf("Tarea 2 ejecutándose en el núcleo 1 %d\n", ban);
    delay(frec/2);
    if (!ban){
      float* values_df = read_file(temp_df, 0, &listSize); // Lee el archivo solo con valores
      float* values_nova = read_file(temp_nova, 0, &listSize); // Lee el archivo solo con valores

      create_file(temp_df);
      create_file(temp_nova);
      //Serial.println("Tarea 2 ejecutándose en el núcleo 1");
      //delay(frec);
      ban = true;
      
      float p_com_df = completeness(values_df, listSize);
      printf("********** Completeness DF: %.5f\n", p_com_df);
      
      float p_com_nova = completeness(values_nova, listSize);
      printf("********** Completeness NOVA: %.5f\n", p_com_nova);

      float m = uncertainty(values_df, values_nova, listSize);
      printf("********** Uncertainty: %.5f\n", m);

      float p_df = precision(values_df, listSize);
      printf("********** Precision DF: %.5f\n", p_df);

      float p_nova = precision(values_nova, listSize);
      printf("********** Precision NOVA: %.5f\n", p_nova);

      int siataList = 1;
      float* value_siata = read_file(siata, siataValue, &siataList);
      float a_df = accuracy(values_df, value_siata[0], listSize);
      printf("********** Accuracy DF: %.5f\n", a_df);

      float a_nova = accuracy(values_nova, value_siata[0], listSize);
      printf("********** Accuracy NOVA: %.5f\n", a_nova);

      float c = PearsonCorrelation(values_df, values_nova, listSize);
      printf("********** Concordance: %.5f\n", c);

      char resultString[50];
      sprintf(resultString, "%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f,%.5f",p_com_df,p_com_nova,p_df,p_nova,a_df,a_nova,m,c);
      write_text_to_file(dimensions, resultString);
      dimen[siataValue%24][0] = p_com_df;
      dimen[siataValue%24][1] = p_com_nova;
      dimen[siataValue%24][2] = p_df;
      dimen[siataValue%24][3] = p_nova;
      dimen[siataValue%24][4] = a_df;
      dimen[siataValue%24][5] = a_nova;
      dimen[siataValue%24][6] = m;
      dimen[siataValue%24][7] = c;

      //read_data_from_file("/spiffs/data.txt"); // Lee el archivo con formato de hora y valor float
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Configurar y conectar WiFi
  ConnectToWiFi();
  
  initialize_spiffs();
  create_file(temp_df); // Archivo de memoria temporal DF
  create_file(temp_nova); // Archivo de memoria temporal DF
  create_file(data); // Archivo de memoria permanente 
  create_file(dimensions); // Archivo que almacena las métricas cada hora 
  write_text_to_file(dimensions, "hora,comp_df,comp_nova,prec_df,prec_nova,acc_df,acc_nova,uncer,concor");

  // Crea dos tareas y las asigna a diferentes núcleos
  xTaskCreatePinnedToCore(task1, "Task1", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(task2, "Task2", 10000, NULL, 1, NULL, 1);
}

void loop() {
    // Tu código en el bucle principal, si es necesario
}