#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "dimensiones.h"
#include "utils.h"

/*
 * Referencias:
 * 1. https://ocw.mit.edu/courses/6-s096-effective-programming-in-c-and-c-january-iap-2014/pages/lecture-notes/
 * 2. https://stackoverflow.com/questions/12911299/read-csv-file-in-c
 * 3. https://stdin.top/posts/csv-in-c/
 * 4. https://github.com/semitrivial/csv_parser
 * 5. https://www.ibm.com/docs/es/i/7.5?topic=functions-strtok-tokenize-string
*/

//#define DEBUG 1  // -DDEBUG=m


float data[] = {3,2,4,6, NAN,1,3,5,6, NAN};

#define BUFER_SIZE 100

FILE* stream;
char buffer[BUFER_SIZE];
char nombre_archivo[] = "../data.csv"; // Modificar con la ruta relativa a donde se genera el ejecutable

int main() {
    hola_mundo();
    hola_mundo();
    printf("Hola, bienvenido a este valle de lagrimas!\n");

    // Inicio: Lectura de un archivo

    stream = fopen(nombre_archivo, "r");
    // If the file exist
    if(stream != NULL) {
        // Archivo existe
        printf("Mirando el archivo en cuestion:\n");
        while(fgets(buffer, BUFER_SIZE, stream)) {
            printf("%s\n", buffer);
        }
    }
    else {
        printf("El archivo no existe\n");
    }

    fclose(stream);
    // Fin: Lectura de un archivo
    printf("Adios mundo cruel!\n");
    int size_data = sizeof(data) / sizeof(int);
    printf("%d\n", size_data);
    float p_com = completitud(data, 10);
    printf("Porcentaje completitud: %.4f", p_com);
    return 0;
}

