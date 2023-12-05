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

//#define FILE_PATH "/spiffs/data.txt"

struct tm get_current_time() {
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    return timeinfo;
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
        printf("Error al montar SPIFFS (%d)\n", ret);
        return;
    }
}

void crear_Archivo(const char *nombreArchivo) {
    
    // Intentar abrir el archivo
    FILE *file = fopen(nombreArchivo, "r");
    if (file != NULL) {
        // El archivo existe, cerrarlo y eliminarlo
        fclose(file);
        if (remove(nombreArchivo) == 0) {
            printf("Archivo existente eliminado: %s\n", nombreArchivo);
        } else {
            printf("Error al eliminar el archivo existente: %s\n", nombreArchivo);
        }
    } else {
        printf("El archivo no existe\n");
    }

    // Crear el archivo
    file = fopen(nombreArchivo, "w");
    if (file == NULL) {
        printf("Error al crear el archivo\n");
        return;
    }

    fclose(file);

}

void write_data_to_file(const char* nombreArchivo, float valor) {
    FILE *file = fopen(nombreArchivo, "a");
    if (file == NULL) {
        printf("Error al abrir el archivo para escritura\n");
        return;
    }

    // Obtener la fecha y hora actual
    struct tm timeinfo = get_current_time();

    fprintf(file, "[%04d-%02d-%02d %02d:%02d:%02d] %.5f\n",
                timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, valor);
    //fprintf(file, "%s", data);
    //printf("Almacenado\n");
    fclose(file);
}

float* leerArchivoSPIFFS(const char* nombreArchivo, int lineaInicio, int *tamanoLista) {
    printf("Abriendo el archivo\n");
    FILE* archivo = fopen(nombreArchivo, "r");

    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return NULL;
    }

    float* lista = (float*)malloc(*tamanoLista * sizeof(float));
    if (lista == NULL) {
        perror("Error al asignar memoria para la lista");
        fclose(archivo);
        return NULL;
    }

    int i = 0;
    int contadorLineas = 0; 
    char linea[256]; // Número de caracteres que desea leer de cada linea, Ajusta el tamaño según tus necesidades
    
    while (fgets(linea, sizeof(linea), archivo) != NULL && i < *tamanoLista) {
        if (contadorLineas >= lineaInicio) {

            if (strcmp(linea, "nan\n") == 0) {
                lista[i] = NAN;  // Representación de NaN en C
            } else {
                sscanf(linea, "%f", &lista[i]);
            }

            i++;
        }
        contadorLineas++;
    }

    *tamanoLista = contadorLineas;
    printf("El tamano de la lista es %d\n", *tamanoLista);
    fclose(archivo);
    //*tamanoLista = i;

    return lista;
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

// Función para leer el archivo y obtener los datos
void read_data_from_file(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        printf("Error al abrir el archivo para lectura\n");
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

                // Obtener la fecha y hora actual
                time_t current_time;
                struct tm *current_timeinfo;

                time(&current_time);
                current_timeinfo = localtime(&current_time);

                // Ajustar la estructura tm con la fecha y hora actual
                entry.timeinfo.tm_year = current_timeinfo->tm_year;
                entry.timeinfo.tm_mon = current_timeinfo->tm_mon;
                entry.timeinfo.tm_mday = current_timeinfo->tm_mday;

                // Hacer algo con la entrada, por ejemplo, imprimir en la consola
                printf("Fecha: %04d-%02d-%02d %02d:%02d:%02d, Valor: %.5f\n",
                       entry.timeinfo.tm_year + 1900, entry.timeinfo.tm_mon + 1, entry.timeinfo.tm_mday,
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


#define MAX_SIZE 60

void readFile() {
    Serial.begin(115200);

    if (!SPIFFS.begin()) {
        Serial.println("Error al montar SPIFFS");
        return;
    }

    File archivo = SPIFFS.open("/datos.txt", "r");

    if (!archivo) {
        Serial.println("Error al abrir el archivo");
        return;
    }

    float lista[MAX_SIZE];
    int i = 0;

    while (archivo.available() && i < MAX_SIZE) {
        // Lee el próximo valor como una cadena
        String valorStr = archivo.readStringUntil('\n');

        // Convierte la cadena a un valor flotante
        if (valorStr.equals("nan")) {
            Serial.println("Se encontró un valor 'nan' en el archivo.");
        } else {
            lista[i] = valorStr.toFloat();
            i++;
        }
        
    }

    archivo.close();

    // Imprime los valores con 5 cifras decimales
    for (int j = 0; j < i; j++) {
        Serial.printf("%.5f\n", lista[j]);
        delay(1000);
    }
}