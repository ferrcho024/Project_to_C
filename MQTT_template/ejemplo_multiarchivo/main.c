#include <string.h>
#include <stdio.h>
#define LOCAL
#include "globals.h"
#include "functions.h"
#include "functions2.h"

int main() {
    printf("Hello, World!\n");
    printf("num = %d\n",num);
    saludo();
    incrementar_num();
    saludo();
    printf("num = %d\n",num);
    strcpy(s,"hola");
    int t = tam(s);
    printf("tam = %d\n",t);
    incrementar_num2(30);
    printf("num = %d\n",num);

    return 0;
}
