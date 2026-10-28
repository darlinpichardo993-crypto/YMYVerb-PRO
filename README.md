# YMYVerb — Vocal Reverb (YMY Records · Made ORYMY)
*The Sweet Space of Bachata.* Reverb vocal VST3 para FL Studio, pensado solo para el espacio vocal de la bachata: predelay sincronizado al tempo, colas cálidas y presets por estilo.

## Estado verificado de esta entrega
| Pieza | Estado |
|---|---|
| Motor DSP (FDN de 8 líneas, predelay, sync, ducking, tono y estéreo) | **Compilado y probado** mediante tests automatizados |
| 34 presets de fábrica (JSON) | Validados: nombres únicos, parámetros reconocidos y valores dentro de rango. Son **puntos de partida por diseño, no calibrados de oído** |
| Plugin JUCE (procesador e interfaz) | **Compilado correctamente en Windows x64** con JUCE 8.0.4 |
| `YMYVerb.vst3` y `YMYVerb_Setup.exe` | **Generados y verificados** en GitHub Actions |
| Pruebas automatizadas Windows | **Aprobadas**: CTest 1/1, con la suite DSP completa |
| Prueba real dentro de FL Studio | **Pendiente**; no debe considerarse validado en un DAW hasta probarlo allí |

**Descarga del último artefacto compilado:** [YMYVerb-PRO-Windows-x64](https://github.com/darlinpichardo993-crypto/YMYVerb-PRO/actions/runs/40669721340/artifacts/12808320068). Contiene el bundle VST3 y el instalador. La compilación confirma que el código se construye y empaqueta; no sustituye una prueba de audio real en FL Studio.

## Cómo obtener el .vst3 y el Setup.exe
### Opción A: descargar el paquete ya compilado
1. Abre [la compilación exitosa en GitHub Actions](https://github.com/darlinpichardo993-crypto/YMYVerb-PRO/actions/runs/40669721340).
2. En **Artifacts**, descarga `YMYVerb-PRO-Windows-x64`.
3. Extrae el ZIP: incluye `YMYVerb_Setup.exe` y la carpeta `YMYVerb.vst3`.

Los pushes y pull requests ejecutan automáticamente la compilación Windows x64, los tests DSP y el empaquetado con Inno Setup.

### Opción B: en tu PC
Instala Visual Studio con la carga de trabajo **Desktop development with C++**, CMake e Inno Setup 6, y ejecuta `scripts\build_windows.bat`. El script detecta una versión compatible de Visual Studio y genera `dist\YMYVerb_Setup.exe` junto a `dist\YMYVerb.vst3`. Requiere Internet para descargar JUCE 8.0.4.

El instalador pone el plugin en `C:\Program Files\Common Files\VST3\YMYVerb.vst3` (requiere administrador por esa carpeta). Luego en FL Studio: Options → Manage plugins → Find plugins.

## Cómo se usa
* **Insert** (en la pista de voz): Dry 100 %, Wet ~25-30 %.  **Aux Send** (canal auxiliar): Dry 0 %, Wet 100 %. Al cambiar el modo en el menú de arriba se aplican esos valores; si estás en Aux con Dry > 0 aparece un aviso.
* **Sync**: el predelay se calcula con el BPM del host (si FL no lo da, usa Manual BPM). Divisiones 1/64 a 1/1, con Triplet o Dotted. Si el resultado pasa de 250 ms se baja por octavas (÷2) para mantenerlo musical. Con Sync apagado, la perilla Predelay es manual (0-250 ms).
* **Micrófono**: se ilumina según el nivel real de entrada (umbral ≈ -50 dB, sube rápido y se apaga suave). Funciona igual en pista o en auxiliar. Botón **LED** lo apaga. No graba ni monitorea audio.
* **Ducking**: baja la reverb mientras hay voz y la deja volver en las pausas (Threshold, Depth, Attack, Release propios).
* **Presets**: categorías, favoritos (Fav), Save/Save As (se guardan en `Documentos\YMY Records\YMYVerb\Presets`, el instalador nunca los toca), A/B, Undo/Redo.
* Perillas: doble clic = valor por defecto.

## Presets (inspirados, NO réplicas)
Romeo Santos · Raulin Rodriguez · Joe Veras (4 cada uno) · Bachata 90s · 2000s · Modern Bachata (largas, 4 cada una) · Aux / Send (10 extra: incluye *Infinite Atmosphere*, decay 14 s). Los nombres de artistas indican el carácter buscado (predelay, calidez, densidad, cola); **no** son sus ajustes reales ni hay vínculo con ellos.

## Pruebas DSP y límites de validación
La suite `Tests/test_dsp.cpp` se compila y ejecuta en GitHub Actions para Windows x64; la ejecución más reciente terminó con **CTest 1/1 aprobado**. Incluye comprobaciones de silencio, tiempo de decaimiento RT60, estabilidad con ajustes extremos, cinco frecuencias de muestreo (44.1–192 kHz), siete tamaños de bloque, predelay, ducking y sincronización de tempo.

**Aún no probado en esta entrega:** escucha de voces reales y ajuste auditivo de presets, consumo de CPU en un Ryzen 7, interacción visual manual, carga dentro de FL Studio, guardado/restauración real de proyectos y validación mediante el VST3 Validator.

## Estructura
`Source/DSP` (motor y sync, sin JUCE) · `Source/PluginProcessor.*` · `Source/PluginEditor.*` · `Source/Presets/factory_presets.json` (generado con `scripts/gen_presets.py`) · `Tests` · `Installer/YMYVerb.iss` · `.github/workflows/build-windows.yml` · `scripts/build_windows.ps1` · `Resources/Interfaz_referencia.png`.

## Diferencias con la imagen de referencia / limitaciones conocidas
* La GUI se dibuja por código (gradientes y vectores), parecida a la referencia pero no idéntica pixel a pixel; no usa la imagen como fondo.
* **Reverb Return** va en dB (−24 a +12), no en %. El nombre del producto es **YMYVerb** (como en la imagen); el prompt original decía "ORYMY Bachata Space".
* Algoritmos: los 6 comparten el mismo motor FDN con parámetros distintos (tamaño, difusión, modulación, reflexiones, brillo); no son 6 estructuras diferentes.
* Auto Makeup es una compensación simple según el decay. Mono entra y sale mono (suma de L/R del efecto).
* Latencia añadida: 0 muestras. La cola de reverb continúa al detener el transporte.

## Licencias
JUCE 8 se usa bajo **AGPLv3** (o licencia comercial de pago). Si distribuyes el plugin compilado, debes publicar el código fuente bajo AGPLv3 o comprar la licencia comercial de JUCE: https://juce.com/legal/juce-8-licence/ . Inno Setup se usa solo para construir el instalador.
