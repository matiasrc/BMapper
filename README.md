# BMapper

Aplicación de video mapping desarrollada con
[openFrameworks](https://openframeworks.cc/) y basada en
[ofxPiMapper](https://ofxpimapper.com/). Permite asignar imágenes, videos,
secuencias PNG y entradas Syphon/Spout a superficies de mapping, con control
por teclado y OSC.

Esta versión está preparada para clases, pruebas y proyectos de mapping en
macOS. El proyecto usa un fork propio de
[ofxPiMapper](https://github.com/matiasrc/ofxPiMapper) con adaptaciones para
BMapper.

## Funciones principales

- Crear y editar superficies rectangulares, triangulares, circulares,
  hexagonales y de grilla.
- Asignar imágenes, videos, secuencias PNG con transparencia y audio.
- Ejecutar contenidos mediante teclas asignadas o mensajes OSC.
- Recibir fuentes Syphon en macOS y Spout en Windows.
- Trabajar en modo edición o presentación, con pantalla completa.
- Guardar el mapping y la configuración OSC de forma protegida, con respaldos
  recuperables.
- Cargar varias secuencias PNG de forma fluida: la decodificación se realiza
  fuera del render y prioriza los fotogramas de todas las fuentes activas.

## Requisitos

- openFrameworks 0.12.0.
- El proyecto debe permanecer en `apps/myApps/BMapper` dentro del árbol de
  openFrameworks, porque el Makefile usa rutas relativas hacia sus librerías y
  addons.
- Los addons declarados en `addons.make`: `ofxGui`, `ofxImGui`, `ofxOsc`,
  `ofxXmlSettings`, `ofxPiMapper`, `ofxSyphon` y `ofxSpout`.

En macOS, Syphon es la entrada de video activa. El código de recepción Spout
está implementado para Windows, pero la compilación Windows debe generarse y
validarse allí antes de considerarse soportada.

## Compilar en macOS con VS Code

1. Abrir `BMapper.code-workspace` en Visual Studio Code.
2. Ejecutar la tarea **Build RELEASE**.
3. La aplicación se abre como `bin/BMapper.app`.

La instalación inicial y los ajustes necesarios de openFrameworks y Syphon
están documentados en [Compilar BMapper en macOS con VS Code](docs/BUILD_MACOS_VSCODE.md).

Usar **Release** para pruebas y clases. El modo Debug puede consumir mucho más
recursos al procesar secuencias grandes.

## Uso rápido

Los contenidos se copian en `bin/data/sources/`:

| Contenido | Carpeta | Formatos |
| --- | --- | --- |
| Imágenes | `imagenes/` | jpg, png, bmp, gif |
| Videos | `videos/` | mp4, mov, avi |
| Secuencias | `secuencias/<nombre>/` | PNG ordenados alfabéticamente |
| Audio | `sonidos/` | wav, aiff, aif, mp3 |

Después de agregar material, elegir **Aplicación → Actualizar recursos**. Para
asignar fuentes, crear o seleccionar una superficie en modo edición y usar el
panel de fuentes.

Atajos principales:

| Acción | Atajo |
| --- | --- |
| Cambiar modo edición | `Cmd/Ctrl + E` |
| Guardar | `Cmd/Ctrl + S` |
| Deshacer | `Cmd/Ctrl + Z` |
| Alternar pantalla completa | `Cmd/Ctrl + F` |
| Entrar en presentación | `Cmd/Ctrl + P` |

En presentación la edición queda bloqueada, pero siguen activos OSC y las
teclas asignadas a los contenidos.

La referencia completa de operación —incluyendo OSC, guardado, recuperación,
transparencia, rendimiento y uso con dos monitores— está en
[Uso operativo de BMapper](docs/USO_OPERATIVO.md).

## Notas importantes

- Las zonas transparentes de una secuencia PNG dejan ver las superficies que
  están por debajo; usar PNG que realmente incluyan canal alfa.
- Dos superficies que usan la misma secuencia comparten reproducción, loop y
  audio. Es intencional para contenidos sincronizados.
- Antes de guardar, BMapper verifica que las fuentes asignadas existan. Si falta
  un archivo, bloquea el guardado para evitar que se pierda la referencia.
- Cada guardado crea una copia de `ofxpimapper.xml` y `mySettings.xml` en
  `bin/data/.backups/`; se conservan las diez más recientes.
- Para proyectar por una segunda pantalla, usar escritorio extendido, mover la
  ventana al proyector y entrar en presentación. Actualmente no hay selector de
  monitor ni una ventana de control independiente.
- En Windows, la fuente externa se presenta como **Spout** e incluye un selector
  nativo de sender. La integración necesita una validación completa en una PC
  Windows antes de distribuir ejecutables para esa plataforma.

## Próximas mejoras

- Elegir y recordar el monitor de proyección.
- Ventana de control independiente de la salida de proyección.
- Importar contenido mediante arrastrar y soltar.
- Filtros y shaders por fuente.
- Recorte y continuidad de una fuente entre varias superficies.

## Créditos

Desarrollado por Matías Romero Costas / Biopus.

Basado en ofxPiMapper, desarrollado originalmente por Krisjanis Rijnieks.
