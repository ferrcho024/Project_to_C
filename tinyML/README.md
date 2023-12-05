# TinyML

## Herramientas necesarias en el PC

Si no se tiene Linux, MAC o WSL es necesario descargar las siguientes aplicaciones para Windows:
1. Git (https://git-scm.com/downloads)
2. Compiladores para windows (https://www.mingw-w64.org/). En nuestro caso, siguiendo las recomendaciones de la sección **Example: Install MinGW-x64 on Windows** de la pagina **C/C++ for Visual Studio Code** ([link](https://code.visualstudio.com/docs/languages/cpp)), se instalo el **Mingw-w64** usando **MSYS2** ([link](https://www.msys2.org/))  https://github.com/msys2/msys2-installer/, este es el link de descarga directo [link](https://github.com/msys2/msys2-installer/releases/download/2023-05-26/msys2-x86_64-20230526.exe). Si sigue las instrucciones al pie de la letra de la pagina de VScode esto dara, en la terminal del MSYS2 se ejecuto el comando:

   ```
   pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain
   ``` 

   Finalmente, se agrego la ruta ```C:\msys64\ucrt64\bin``` al ```PATH``` de Windows.
3. Creo que se puede instalar con la herramienta anterior (buscar como). Pero en el caso hecho en esta maquina, lo que se hizo fue usar **chocolatey** ([link](https://chocolatey.org/)). Una vez instalado en windows, se ejecuto en la terminal el comando como administrador el comando: 
   
   ```
   choco install make
   ```

## Test de las herramientas

En la terminal, se verifican la existencia de las nuevas herramientas usando los comandos:

```
gcc --version
g++ --version
gdb --version
make --version
```

## Sobre los Notebooks de python

Lo ideal es que esto se haga en [colab](https://colab.google/) y no localmente.

## Proceso

Como punto de partida se siguen los ejemplos realizados en el libro **TinyML** ([link](https://tinymlbook.com/)), cuyos archivos originales esta en el repo: https://github.com/tensorflow/tensorflow/tree/be4f6874533d78f662d9777b66abe3cdde98f901/tensorflow/lite/experimental/micro/examples, sin embargo, la ultima versión esta en el repo: https://github.com/tensorflow/tflite-micro




