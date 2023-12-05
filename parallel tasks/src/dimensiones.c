//
// Created by Usuario on 12/4/2023.
//
#include <math.h>
#include <stdio.h>
#include "dimensiones.h"

// Importante mirar para cuando se saquen los datos
float completitud(float *data, int size) {
    int datos_esperados = size;
    int datos_validos = 0;
    for (int i = 0; i < size; i++) {
        if (!isnan(*(data + i))) {
            datos_validos++;
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
    return (float) datos_validos / datos_esperados;
}

float incertidumbre(float *data1,float *data2, int size) {
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