# Shadertext Editor

## Tabla de Contenidos

- [Características](#características)
- [Requisitos](#requisitos)
- [Instalación](#instalación)
- [Configuración](#configuración)
- [Licencia](#licencia)

## Características

## Requisitos

- **SDL2 2.0.12+** ([Official Site](https://www.libsdl.org/))
- **FreeType 2.13.0+** ([Documentación](https://freetype.org/))
- **GLEW 2.1.0+** ([Repositorio](https://github.com/nigels-com/glew))
- Compilador MSVC o compatible C11

# Instalación

# ⚙ Windows (MSVC)

## Descargar dependencias

```bash
.\setup_dependencies.bat
```

## Compilar proyecto

```bash
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
```

```bash
.build_msvc.bat
```

## Ejecutar editor

```bash
.\ded.exe src\main.c
```

## Configuración

- Desktop Development with C++ workload
- Windows 10 SDK (10.0.19041.0)
- Requiere fuentes en el directorio `fonts/`
  - Victor Mono - Fuente monospace principal
  - Iosevka - Fuente monospace secundaria

## Licencia

MIT License - Ver [LICENSE]() para más detalles

## Contribuciones

1. Clonar repositorio
2. Crear rama para nueva funcionalidad o corrección de errores
   `git checkout -b feature-1/mejora`
3. Hacer commit de cambios
   `git commit -m 'Add amazing feature'`
4. Push a la rama
   `git push origin feature-1/mejora`
5. Abrir Pull Request y esperar una revisión


