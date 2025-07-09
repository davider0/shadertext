@echo off
rem launch this from msvc-enabled console

set CFLAGS=/W4 /WX /wd4244 /wd4305 /wd4018 /wd4702 /std:c11 /wd4996 /wd5105 /wd4200 /wd4013 /FC /TC /Zi /nologo /EHsc
set INCLUDES=/I dependencies\SDL2\include ^
             /I dependencies\GLFW\include ^
             /I dependencies\GLEW\include ^
             /I dependencies\freetype ^
             /I dependencies\freetype\config ^
             /I dependencies\freetype\freetype\config ^
             /I src ^
             /I dependencies\freetype\freetype\lib

set LIBS=dependencies\SDL2\lib\x64\SDL2.lib ^
         dependencies\SDL2\lib\x64\SDL2main.lib ^
         dependencies\GLFW\lib\glfw3.lib ^
         dependencies\GLEW\lib\glew32s.lib ^
         "C:\Users\David\Desktop\ded-master\dependencies\freetype\freetype\lib\freetype.lib"

cl.exe %CFLAGS% %INCLUDES% /Feded src\main.c src\la.c src\editor.c src\file_browser.c src\free_glyph.c src\simple_renderer.c src\common.c /link %LIBS% -SUBSYSTEM:windows 