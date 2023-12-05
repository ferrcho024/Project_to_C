//
// Created by Usuario on 12/4/2023.
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "test.h"
#include "funciones.h"
#include "dimensiones.h"

void test_archivos(void) {
    int lineaInicio = 0, tamanoLista = 0;
    float *miLista;
    tamanoLista = 10;
    lineaInicio = lineaInicio;
    // lectura 1
    printf("------------- data[0:10] -> cache_data1 -------------\n");
    miLista = leer_Archivo("data.txt", lineaInicio, tamanoLista);
    write_data_to_file("cache_data1.txt",miLista,tamanoLista);
    free(miLista); // Se libera el heap

    // lectura 2
    printf("------------- data[10:20] -> cache_data2 -------------\n");
    lineaInicio = lineaInicio + tamanoLista;
    miLista = leer_Archivo("data.txt", lineaInicio, tamanoLista);
    write_data_to_file("cache_data2.txt",miLista,tamanoLista);
    free(miLista); // Se libera el heap
}

void test_completitud(void) {
    float data[] = {3,2,4,6, NAN,1,3,5,6, NAN};
    float p_com = completitud(data, 10);
    printf("Porcentaje completitud: %.5f", p_com);
}