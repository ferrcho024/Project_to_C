#include <FS.h>
#include <SPIFFS.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_spiffs.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"
//#include "lwip/apps/sntp.h"

#include "WiFi.h" // ESP32 WiFi include
#include "esp_sntp.h"
#include "WiFiConfig.h" // My WiFi configuration.


void printTime(){
  
  struct tm time;
   
  if(!getLocalTime(&time)){
    Serial.println("Could not obtain time info");
    return;
  }
 
  Serial.println("\n---------TIME----------");
  Serial.println(&time, "%A, %B %d %Y %H:%M:%S");
  Serial.println("");
   
//   Serial.print("Number of years since 1900: ");
//   Serial.println(time.tm_year);
 
//   Serial.print("month, from 0 to 11: ");
//   Serial.println(time.tm_mon);
 
//   Serial.print("day, from 1 to 31: "); 
//   Serial.println(time.tm_mday);
 
//   Serial.print("hour, from 0 to 23: ");
//   Serial.println(time.tm_hour);
 
//   Serial.print("minute, from 0 to 59: ");
//   Serial.println(time.tm_min);
   
//   Serial.print("second, from 0 to 59: ");
//   Serial.println(time.tm_sec);

}

void ConnectToWiFi()
{
 
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, WiFiPassword);
  Serial.print("Connecting to "); Serial.println(SSID);
 
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(500);
 
    if ((++i % 16) == 0)
    {
      Serial.println(F(" still trying to connect"));
    }
  }
 
  Serial.print(F("Connected. My IP address is: "));
  Serial.println(WiFi.localIP());

  delay(1000);

  // Configurar el servicio SNTP
  configTime(-18000, 3600, ntpServer); // -18000 es para UTC -5 (-5*60*60)

  printTime();
  delay(1000);
}

struct tm get_current_time() {

    struct tm time;
   
    if(!getLocalTime(&time)){
        Serial.println("Could not obtain time info");
        return time;
    }

    return time;
}

void initialize_spiffs() {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        printf("SPIFFS mount error (%d)\n", ret);
        return;
    }
}

void create_file(const char *file_path) {
    
    // Intentar abrir el file
    FILE *file = fopen(file_path, "r");
    if (file != NULL) {
        // El file existe, cerrarlo y eliminarlo
        fclose(file);
        if (remove(file_path) == 0) {
            printf("Existing file deleted: %s\n", file_path);
        } else {
            printf("Error deleting the existing file: %s\n", file_path);
        }
    } else {
        printf("File does not exist\n");
    }

    // Crear el file
    file = fopen(file_path, "w");
    if (file == NULL) {
        printf("Error creating the file\n");
        return;
    }

    fclose(file);

}

void write_data_to_file(const char* file_path, float value) {
    FILE *file = fopen(file_path, "a");
    if (file == NULL) {
        printf("Error opening the file for writing\n");
        return;
    }

    // Obtener la fecha y hora actual
    struct tm timeinfo = get_current_time();

    fprintf(file, "[%04d-%02d-%02d %02d:%02d:%02d] %.5f\n",
                timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, value);

    fclose(file);

    // Almacenamiento en el file temporal. En este se guarda solo el dato sin fecha para facilidad en la extraccion de los datos para los cálculos
    FILE *file2 = fopen("/spiffs/temp.txt", "a");
    if (file2 == NULL) {
        printf("Error opening the file for writing\n");
        return;
    }

    fprintf(file2, "%.5f\n", value);

    fclose(file2);

}

float* read_file(const char* file_path, int startLine, int *listSize) {
    FILE* file = fopen(file_path, "r");

    if (file == NULL) {
        perror("Error al abrir el file");
        return NULL;
    }

    float* values = (float*)malloc(*listSize * sizeof(float));
    if (values == NULL) {
        perror("Error allocating memory for the list");
        fclose(file);
        return NULL;
    }

    int i = 0;
    int linesCounter = 0; 
    char line[256]; // Número de caracteres que desea leer de cada linea, Ajusta el tamaño según tus necesidades
    
    while (fgets(line, sizeof(line), file) != NULL && i < *listSize) {
        if (linesCounter >= startLine) {

            if (strcmp(line, "nan\n") == 0) {
                values[i] = NAN;  // Representación de NaN en C
            } else {
                sscanf(line, "%f", &values[i]);
            }

            i++;
        }
        linesCounter++;
    }

    *listSize = linesCounter - startLine;
    
    fclose(file);
    //*listSize = i;

    return values;
}

struct DataEntry {
    struct tm timeinfo;
    float value;
};

// Función para convertir una cadena de fecha y hora a una estructura tm
int parse_date_time(const char *str, struct tm *timeinfo) {
    return sscanf(str, "[%d-%d-%d %d:%d:%d]", 
                  &timeinfo->tm_year, &timeinfo->tm_mon, &timeinfo->tm_mday,
                  &timeinfo->tm_hour, &timeinfo->tm_min, &timeinfo->tm_sec);
}

// Función para leer el file y obtener los datos
void read_data_from_file(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        printf("Error al abrir el file para lectura\n");
        return;
    }

    char line[256];
    struct DataEntry entry;

    while (fgets(line, sizeof(line), file) != NULL) {
        // Parsear la línea para obtener fecha, hora y valor float
        if (parse_date_time(line, &entry.timeinfo) == 6) {
            char *value_str = strchr(line, ']');
            if (value_str != NULL) {
                value_str += 2;  // Apuntar al espacio después del ']'
                sscanf(value_str, "%f", &entry.value);

                // Hacer algo con la entrada, por ejemplo, imprimir en la consola
                printf("Fecha: %04d-%02d-%02d %02d:%02d:%02d, Valor: %.5f\n",
                       entry.timeinfo.tm_year, entry.timeinfo.tm_mon, entry.timeinfo.tm_mday,
                       entry.timeinfo.tm_hour, entry.timeinfo.tm_min, entry.timeinfo.tm_sec,
                       entry.value);
            } else {
                printf("Error al extraer el valor float de la línea: %s\n", line);
            }
        } else {
            printf("Error al parsear la línea: %s\n", line);
        }
    }

    fclose(file);
}



// Dimeniones
float completeness(float *data, int size) {
    int expectedValues = size;
    int validValues = 0;
    for (int i = 0; i < size; i++) {
        if (!isnan(*(data + i))) {
            validValues++;
            // Dato presente
#ifdef DEBUG
            printf("Data[%d] = %.4f\n", i,*(data + i));
#endif
        }
#ifdef DEBUG
        else {
            // Dato no disponible
            printf("Data[%d] = nan\n", i);
        }
#endif
    }
    return (float) validValues / expectedValues;
}

float uncertainty(float *data1,float *data2, int size) {
    float error = 0, d1, d2;
    float avg = 0;
    float v;
    float max;
    for (int i = 0; i < size; i++) {
        // Data 1
        if(isnan(*(data1 + i))) {
            d1 = 0;
        }
        else {
            d1 = *(data1 + i);
        }
        // Data 2
        if(isnan(*(data2 + i))) {
            d2 = 0;
        }
        else {
            d2 = *(data2 + i);
        }
        error += (d1 - d2)*(d1 - d2);
        avg += (d1 + d2);
#ifdef DEBUG
    printf("%10.5f - %10.5f --- %10.5f --- %10.5f\n", d1, d2, (d1 - d2)*(d1 - d2), (d1 + d2));
#endif
    }
    avg /= 2*size;
    v = sqrt(error/(2*size*avg*avg));
#ifdef DEBUG
    printf("------------------------------------------------------\n");
    printf("Error =  %.5f; Promedio =  %.5f\n", error, avg);
    printf("v = %.5f\n",v);
#endif
    if (0 >= 1 - v) {
        max = 0;
    }
    else {
        max = 1-v;
    }
    return max;
}