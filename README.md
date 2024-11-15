# BMapper
Software para video mapping, realizado con [OpenFrameworks](https://openframeworks.cc/), basdo en [ofxPiMapper] (https://ofxpimapper.com/) desarrollado por Krisjanis Rijnieks. 


## DEPENDENCIES ##

ofxGui (included in Of Core)

ofxOsc (Incluido en OF / included in OF Core)

ofxXmlSettings (included in Of Core)

[ofxSpout] (https://github.com/elliotwoods/ofxSpout)

[ofxSyphon] (https://github.com/astellato/ofxSyphon)

[ofxImGui] (https://github.com/jvcleave/ofxImGui)

[ofxPiMapper fork] (https://github.com/matiasrc/ofxPiMapper/tree/main) 


## Ayuda ##
# MODOS #
                        
Modo Edición (Control / Cmd + e): Cambia de modo para poder crear, editar superficies y definir su contenido.\n\n"
                        
# CONTENIDOS #
Para agregar contenidos, ubicarlos en la carpeta 'data/fuentes'. Se pueden agregar imágenes (jpg, png, bmp, gif), videos (mov, mp4, avi), secuencias de PNG con transparencia, sonidos (wav, mp3, aiff, aif).

# ACCESO RÁPIDO DE TECLADO #
Modo edición: ctrl / cmd + e
Guardar: ctrl / cmd + s
Deshacer: ctrl / cmd + z
Pantalla completa: ctrl / cmd + f
Modo presentación: ctrl / cmd + p
Modificar superficies:
ocultar o ver capas:
                        
                    
Agrandar superficie seleccionada: +
Achicar superficie seleccionada: -                          
Mover punto, o superficies: flecha o flechas + shift
Seleccionar superficie siguiente:  .
Seleccionar superficie anterior:  ,

eleccionar vértice siguiente:  <
Seleccionar vértice anterior:  >
Solo en modo EDICIÓN: \

Crear superficie triangular: t
Crear superficie rectangular: q
Crear superficie circular: c
Crear superficie hexagonal: h
Crear superficie grilla: g
Duplicar superficie: d
Borrar superficie: backspace
                        
# CONTROL DE CONTENIDOS #
Tanto los videos como las secuencias de png se pueden ejecutar con los comandos: play, stop, pause y resume, tanto desde el menú de cada superficie como desde afuera a través de mensajes OSC.
También se pueden reproducir (play) a partir de una tecla del teclado.
                        
OSC permite enviar mensajes a una superficie, usando etiquetas definidas para comandos como '/superficie1 play'. 

Lo mismo puede hacerse con el resto de los controles:
                        "/superficie1 stop\n"
                        "/superficie1 pause\n"
                        "/superficie1 resume \n"

### TODO ###

- Help
- Drag and drop de contenidos en las superficies
- Entrada por NDI
- Filtros por Shaders

## PRE RELEASES ##
[Binaries / ejecutables](https://github.com/matiasrc/BBlobTracker-lite/releases/tag/v.0.1) OSX y WIN
