//
// Created by Usuario on 12/4/2023.
//
#include <math.h>
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