#include <FS.h>
#include <SPIFFS.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_spiffs.h"

float* leerArchivoSPIFFS(const char* nombreArchivo, int lineaInicio, int tamanoLista) {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        printf("Error al montar SPIFFS (%d)\n", ret);
        return NULL;
    }

    FILE* archivo = fopen(nombreArchivo, "r");

    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return NULL;
    }

    float* lista = (float*)malloc(tamanoLista * sizeof(float));
    if (lista == NULL) {
        perror("Error al asignar memoria para la lista");
        fclose(archivo);
        return NULL;
    }

    int i = 0;
    int contadorLineas = 0;

    char linea[256]; // Número de caracteres que desea leer de cada linea, Ajusta el tamaño según tus necesidades
    while (fgets(linea, sizeof(linea), archivo) != NULL && i < tamanoLista) {
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

    fclose(archivo);
    //*tamanoLista = i;

    return lista;
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