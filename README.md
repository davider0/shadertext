# 🎨 Shadertext Editor

## Tabla de Contenidos

- [📃 Características](#features)
- [🔧 Setup ](#setup)
- [💿 Compilación](#build)
- [📄 Licencia](#license)
- [✍ Contribuciones](#contributions)

## Características <a id="features"></a>

Editor de texto minimalista en C, sin dependencias externas (solo el estándar de C y make) hecho en un sólo archivo C.

## Setup <a id="setup"></a>

### Requisitos básicos

- **Compilador de C** (`cc`)
- **Herramienta `make`**  
   Comprueba si están instalados ejecutando:
  ```bash
  cc --version
  make -v
  ```

### Instalación por sistema operativo

- **Windows**  
  Requiere un entorno Linux dentro de Windows, ya que el editor usa `<termios.h>`. Opciones recomendadas:

  - **Bash on Windows** (solo Windows 10 64-bit): tras instalarlo, ejecuta:
    ```bash
    sudo apt-get install gcc make
    ```
  - **Cygwin**: durante la instalación, selecciona los paquetes `gcc-core` y `make` desde la categoría `devel`. Luego usa el terminal de Cygwin para compilar

- **macOS**  
  Ejecuta:

  ```bash
  xcode-select --install
  ```

  Esto instalará un compilador de C y `make`, como parte de las Command Line Tools

- **Linux (ej. Ubuntu)**  
   Ejecuta:
  ```bash
  sudo apt-get install gcc make
  ```

## Compilación <a id="build"></a>

### Paso 1: Crear el archivo fuente

Crea un archivo llamado `shadertext.c`:

```c
int main() {
  return 0;
}
```

### Paso 2: Compilar desde la terminal

```bash
cc shadertext.c -o shadertext
./shadertext
echo $?
```

El comando `echo $?` debería devolver `0`, indicando ejecución exitosa

### Paso 3: Usar `make`

Crea un archivo llamado `Makefile` con este contenido:

```makefile
shadertext: shadertext.c
   $(CC) shadertext.c -o shadertext -Wall -Wextra -pedantic -std=c99
```

Luego ejecuta:

```bash
make
```

Esto compilará tu programa automáticamente :contentReference[oaicite:6]{index=6}.

## Licencia <a id="license"></a>

MIT License – Consulta el archivo [LICENSE](LICENSE) para más detalles.

## Contribuciones <a id="contributions"></a>

1. Clona el repositorio.
2. Crea una rama para tu cambio:
   ```bash
   git checkout -b feature/nueva-funcionalidad
   ```
3. Haz commit de tus cambios:
   ```bash
   git commit -m "Agregar una funcionalidad increíble"
   ```
4. Envía tu rama:
   ```bash
   git push origin feature/nueva-funcionalidad
   ```
5. Abre un Pull Request y espera la revisión.

> _“Haz un programa que sea tan simple que obviamente no tenga deficiencias o un programa tan complejo que no tenga deficiencias obvias.”_ – C. A. R. Hoare
