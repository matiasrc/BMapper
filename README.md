# BMapper
Software para video mapping, realizado con [OpenFrameworks](https://openframeworks.cc/), basado en [ofxPiMapper](https://ofxpimapper.com/) desarrollado por Krisjanis Rijnieks. 


## Dependencias

ofxGui (included in Of Core)

ofxOsc (Incluido en OF / included in OF Core)

ofxXmlSettings (included in Of Core)

[ofxSpout](https://github.com/elliotwoods/ofxSpout)

[ofxSyphon](https://github.com/astellato/ofxSyphon)

[ofxImGui](https://github.com/jvcleave/ofxImGui)

[ofxPiMapper fork](https://github.com/matiasrc/ofxPiMapper/tree/main) 


## Ayuda
                        
### Contenidos
Los contenidos (imagenes, videos, secuencias, sonidos) deben agregarse en la carpeta 'data/sources'. Se pueden agregar imágenes (jpg, png, bmp, gif), videos (mov, mp4, avi), secuencias de PNG con transparencia, sonidos (wav, mp3, aiff, aif). Una vez agradados, al abrirse nuevamente la aplicaión, estarán disponibles para ser asignados a una superficie. 

### Accesos rápidos de teclaro
#### Generales
Función | tecla
:--- | :---
Modo edición | ctrl / cmd + e  
Guardar | ctrl / cmd + s  
Deshacer | ctrl / cmd + z  
Pantalla completa | ctrl / cmd + f  
Modo presentación | ctrl / cmd + p  

#### Modificar superficies
Función | tecla
:--- | :---
Ocultar o ver capas | l  
Agrandar superficie seleccionada | +  
Achicar superficie seleccionada  | -                            
Seleccionar superficie siguiente |  .  
Seleccionar superficie anterior |  ,  
Seleccionar vértice siguiente |  <  
Seleccionar vértice anterior |  >  

Mover vértice o superficies (una vez seleccionados): flecha o flechas + shift (para hacerlo por pasos más grandes)

#### Solo en modo EDICIÓN:
Función | tecla
:--- | :---
Crear superficie triangular | t  
Crear superficie rectangular | q  
Crear superficie circular | c  
Crear superficie hexagonal | h  
Crear superficie grilla | g  
Duplicar superficie | d  
Borrar superficie | backspace  
                        
### Control de contenidos
Tanto los videos como las secuencias de png se pueden ejecutar con los comandos: play, stop, pause y resume, tanto desde el menú de cada superficie como desde afuera a través de mensajes OSC.  
También se pueden reproducir (play) a partir de una tecla del teclado.  
                        
OSC permite enviar mensajes a una superficie, usando etiquetas definidas para comandos como `/superficie1 play`   

Lo mismo puede hacerse con el resto de los controles:  
                        `/superficie1 stop`  
                        `/superficie1 pause`     
                        `/superficie1 resume`      

## TODO

- Help
- Drag and drop de contenidos en las superficies
- Entrada por NDI
- Filtros por Shaders

## PRE RELEASES
[Binaries / ejecutables](https://github.com/matiasrc/BMapper/releases/tag/v.0.1) OSX y WIN
