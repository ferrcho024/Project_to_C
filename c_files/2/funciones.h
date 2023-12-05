//
// Created by Usuario on 12/4/2023.
//

#ifndef ENSAYOS_C2_FUNCIONES_H
#define ENSAYOS_C2_FUNCIONES_H

void crear_Archivo(const char *nombreArchivo);
float* leer_Archivo(const char* nombreArchivo, int lineaInicio, int tamanoLista);
void write_data_to_file(const char* nombreArchivo, float *buffer, int size);

#endif //ENSAYOS_C2_FUNCIONES_H
