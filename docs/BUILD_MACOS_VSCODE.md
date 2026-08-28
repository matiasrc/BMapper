# Compilar BMapper en macOS con VS Code

Esta guía corresponde al entorno validado el 28 de agosto de 2026:

- openFrameworks 0.12.0 para macOS.
- Este proyecto ubicado en `apps/myApps/BMapper` dentro del árbol de openFrameworks.
- El fork `matiasrc/ofxPiMapper`, en `addons/ofxPiMapper`.

No mover BMapper fuera de `apps/myApps`: el `Makefile` generado usa rutas
relativas hacia `../../../libs/openFrameworks` y `../../../addons`.

## Abrir y compilar

1. Abrir `BMapper.code-workspace` en Visual Studio Code.
2. Ejecutar la tarea **Build Release** (o, desde la carpeta de BMapper,
   `make Release`).
3. Ejecutar la tarea **Run Release** o abrir `bin/BMapper.app`.

El `Makefile`, `config.make` y `.vscode/` del repositorio son la configuración
generada para este flujo. Si se vuelve a ejecutar Project Generator, revisar
antes de confirmar cambios que estas rutas sigan siendo correctas.

## Ajuste necesario en ofxSyphon

La versión instalada de ofxSyphon necesita compilar su implementación Objective-C
y copiar el framework dentro de la aplicación. En
`addons/ofxSyphon/addon_config.mk`, dentro de la sección `osx:`, deben existir
estas líneas:

```make
ADDON_INCLUDES += $(OF_ROOT)/addons/ofxSyphon/libs/Syphon/src
ADDON_SOURCES += libs/Syphon/src/SyphonNameboundClient.m
ADDON_AFTER = mkdir -p bin/$(BIN_NAME).app/Contents/Frameworks && cp -R $(OF_ROOT)/addons/ofxSyphon/libs/Syphon/lib/osx/Syphon.framework bin/$(BIN_NAME).app/Contents/Frameworks/
```

Las líneas `ADDON_CFLAGS` y `ADDON_LDFLAGS` que ya estaban en ese archivo se
mantienen. El último ajuste hace que `BMapper.app` pueda encontrar Syphon al
arrancar, además de enlazarlo al compilar.

## Ajuste necesario en openFrameworks

En SDKs actuales de macOS no existe el framework `AGL`. En
`libs/openFrameworksCompiled/project/osx/config.osx.default.mk`, eliminar esta
línea si está presente:

```make
PLATFORM_FRAMEWORKS += AGL
```

No es un cambio de BMapper ni de PiMapper: pertenece a esa copia local de
openFrameworks 0.12.0. Si se instala otra versión de openFrameworks, verificar
su configuración en vez de copiar este ajuste a ciegas.

## Verificación mínima

Después de una compilación nueva, comprobar:

1. La aplicación abre desde VS Code.
2. Carga un archivo de mapping y sus fuentes.
3. Recibe el OSC esperado.
4. Puede seleccionar una fuente Syphon.

Los archivos `bin/data/ofxpimapper.xml` y `bin/data/BMapper_OSC/BMapper_OSC.pde`
son datos de trabajo y pueden cambiar durante una sesión. No se incluyen en los
commits de configuración salvo que se quiera publicar intencionalmente un preset.
