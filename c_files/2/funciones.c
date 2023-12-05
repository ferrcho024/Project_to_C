//
// Created by Usuario on 12/4/2023.
//
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "funciones.h"

void write_data_to_file(const char* nombreArchivo, float *buffer, int size) {
    FILE *file = fopen(nombreArchivo, "w");
    if (file == NULL) {
        printf("Error al abrir el archivo para escritura\n");
        return;
    }
    //printf("Lista de valores:\n");
    // Escritura del buffer
    for (int i = 0; i < size; i++) {
        if(isnan(*(buffer + i))) {
            fprintf(file, "%s\n", "nan");
        }
        else {
            fprintf(file, "%.5f\n", *(buffer + i));
        }
    }
    //fprintf(file, "%s", data);
    //printf("Almacenado\n");
    fclose(file);
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


float* leer_Archivo(const char* nombreArchivo, int lineaInicio, int tamanoLista) {
    // printf("Abriendo el archivo\n");
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
                //printf("%f\n", lista[i]);
            }

            i++;
        }
        contadorLineas++;
    }
    // printf("Lineas leidas: %d\n",contadorLineas);
    fclose(archivo);
    //*tamanoLista = i;

    return lista;
}
