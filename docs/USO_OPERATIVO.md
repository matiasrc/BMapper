# Uso operativo de BMapper

Esta guía describe la versión de BMapper validada para macOS con openFrameworks
0.12.0 y el fork local de ofxPiMapper.

## Inicio y modos

Abrir `BMapper.code-workspace` en VS Code y ejecutar **Build RELEASE**. La
configuración detallada de compilación está en [BUILD_MACOS_VSCODE.md](BUILD_MACOS_VSCODE.md).

La aplicación tiene dos modos:

- **Edición** (`Cmd/Ctrl + E`): permite crear, modificar y guardar superficies.
- **Presentación** (`Cmd/Ctrl + P`): bloquea la edición y deja activos OSC y las
  teclas asignadas a contenidos. También entra en pantalla completa.

`Cmd/Ctrl + F` alterna pantalla completa sin cambiar el modo. `Cmd/Ctrl + S`
guarda el proyecto y `Cmd/Ctrl + Z` deshace la última acción de mapping mientras
se está en edición.

## Fuentes

Los recursos se cargan desde `bin/data/sources/`:

| Tipo | Carpeta | Formatos admitidos |
| --- | --- | --- |
| Imágenes | `imagenes/` | jpg, png, bmp, gif |
| Videos | `videos/` | mp4, mov, avi |
| Secuencias | `secuencias/<nombre>/` | PNG ordenados alfabéticamente |
| Audios | `sonidos/` | wav, aiff, aif, mp3 |

Después de copiar material nuevo, usar **Aplicación → Actualizar recursos**.
No renombrar, mover ni borrar un archivo que ya esté asignado a una superficie:
BMapper lo detecta y bloquea el guardado hasta que se restaure el recurso o se
reasigne esa superficie.

Una secuencia conserva el alfa de sus PNG: sus zonas transparentes permiten ver
las superficies inferiores. Para asegurar ese resultado, usar PNG con canal alfa
real y probarlas superpuestas a una imagen u otra superficie.

Las superficies que eligen la misma secuencia usan la misma fuente. Por eso
comparten reproducción, pausa, loop y audio; es el comportamiento actual,
intencional y eficiente para contenidos sincronizados.

## Reproducción y rendimiento

Al dar Play a una secuencia, BMapper prepara unos fotogramas antes de iniciar la
imagen y el audio. Mientras está preparando se muestra el estado en la interfaz.
Los PNG se decodifican fuera del render y los fotogramas próximos de todas las
secuencias activas se atienden con prioridad equilibrada. Esto evita que una
secuencia larga interrumpa a las demás.

La resolución interna actual de las fuentes de secuencia es 800×600. Es una
decisión de rendimiento apropiada para ejercicios y pruebas, pero no reemplaza
una futura configuración por perfil de proyector.

## OSC

El puerto de entrada se ajusta desde **OSC → Puerto OSC**; el valor inicial es
3333. Cada superficie puede tener una dirección OSC propia. BMapper espera:

```
/direccion-de-la-superficie  play
/direccion-de-la-superficie  pause
/direccion-de-la-superficie  stop
/direccion-de-la-superficie  resume
```

El comando es el primer argumento de texto del mensaje OSC. Los controles de
reproducción se aplican a videos y secuencias; una imagen fija no responde a
`play`, `pause`, `stop` ni `resume`.

## Guardado y recuperación

Al guardar, BMapper verifica que los recursos asignados sigan disponibles. Si
la verificación es correcta, guarda `ofxpimapper.xml` y `mySettings.xml`.
Antes de reemplazarlos crea un respaldo en `bin/data/.backups/`; conserva los
10 más recientes. En modo edición también hay autoguardado cada minuto.

Para recuperar una configuración, cerrar BMapper y copiar los dos XML de una
carpeta de respaldo sobre los archivos equivalentes de `bin/data/`.

## Uso con dos monitores

BMapper usa una sola ventana. Para proyectar sólo por la segunda salida, el
escritorio debe estar configurado como pantallas extendidas: mover la ventana al
proyector y usar `Cmd/Ctrl + P` o `Cmd/Ctrl + F`. La primera pantalla queda libre
para el escritorio u otras aplicaciones.

Actualmente BMapper no permite elegir y recordar un monitor desde su interfaz,
ni mantiene una ventana de control propia en una pantalla mientras proyecta en
otra. Son mejoras futuras separadas de la versión actual.
