# Guion del video demostrativo (3 a 5 minutos, duración objetivo: 4:15)

Sin voz: cada tramo lleva un **rótulo** (texto en pantalla) que dice qué requisito se está viendo. Si prefieren narrar, el rótulo sirve de guion.
Grabar a 1080p con OBS (o la grabadora de pantalla de Windows, Win+Alt+R), a pantalla completa, con el `.exe` instalado.

## Antes de grabar
- Hacer un recorrido completo del relato una vez para descartar errores.
- Cerrar otras ventanas y notificaciones. Si el programa pide pantalla completa, abrirlo así.
- Ensayar la secuencia de teclas abajo; el video no necesita estar corrido de una sola toma, se puede cortar por tramos.
- Evitar pulsar **M** en la escena 3 (en una prueba en Linux se colgó ahí una vez); úsenlo en la escena 9.
- Navegación entre escenas: **AvPag / RePag** (o **]** y **[**).

## Secuencia

| Tiempo | Qué se ve | Teclas | Rótulo (requisito) |
|---|---|---|---|
| 0:00–0:15 | Portada y escena 0: "2 de octubre de 1968" sobre la plaza | abrir programa, **ENTER** | "T68 Sci-Fi Tribute · Equipo Gundam" / "Tema: México y su cultura" |
| 0:15–0:40 | Escena 1: obreros martillan, CUAU despierta | **AvPag**, **ESPACIO** ×3 (martillazos), **ESPACIO** (despertar) | "Animación por código: keyframes (T6, T11)" / "Skybox (T4)" |
| 0:40–1:00 | Escena 3: CUAU levanta la MAC con sus sierras girando | **AvPag** ×2, **ESPACIO** | "Animación básica: rotación de aspas (T6, T11)" |
| 1:00–1:20 | Escena 4: CUAU esquiva a los SENTINEL | **AvPag**, **←**, **→**, **←** | "Animación procedural: zigzag y barrido (T6, T11)" |
| 1:20–1:40 | Escena 8: CUAU derrota a los SENTINEL y celebra | **AvPag** ×4, **ESPACIO** | "Animación de huesos: Victory de Mixamo (T5)" |
| 1:40–2:05 | Escena 9: la ofrenda con papel picado ondeando y pétalos cayendo | **AvPag**, **ENTER** (textos) | "Animación de Blender: papel picado y pétalos (T7)" |
| 2:05–2:25 | Recorrido de cámara por la iglesia | **M** | "Animación de Blender: recorrido de cámara (T7)" |
| 2:25–3:00 | Iluminación y materiales: acercarse a CUAU y alternar | **F2**, **WASD** + mouse, **E** (apagar y encender), **L**, **T**, **H** | "Phong + Fresnel + environment mapping (T9, T10)" / "Luces interactivas: linterna, casco, día/noche (T3)" |
| 3:00–3:55 | Modo libre: panel de controles abierto y las 8 animaciones por input | **2**, **3**, **4**, **5** (y **ESPACIO**), **8**, **9**, **click izq**, **click der**; dejar ~6 s cada una | "8 animaciones por input (T8)" (cambiar el rótulo con cada una: "2 Barrido", "3 Zigzag", "4 Caminata por partes", "5 Obreros", "8 Cae un SENTINEL", "9 La MAC levita", "Click izq: CUAU salta", "Click der: los SENTINEL giran") |
| 3:55–4:05 | Extras de huesos en modo libre | **6**, **7** | "Animaciones de huesos adicionales (T5)" |
| 4:05–4:15 | Cierre: créditos | (pantalla de rótulos) | "Modelos: Santiago Velasco, Victor Duque, Erick Echeverria · Animaciones: Mixamo · Texturas: Poly Haven · Railgun: samuel2wde (CC-BY)" |

## Qué requisito aparece en cada tramo (para el reporte)

| Requisito | Dónde se ve |
|---|---|
| T1, T2 (modelos) | Todo el recorrido; el inventario está en `docs/inventario_modelos.md` |
| T3 (luces) | 2:25–3:00 |
| T4 (skybox, texturizado) | 0:15 en adelante |
| T5 (huesos) | 1:20 y 3:55 |
| T6, T11 (código: básica, keyframes, procedural) | 0:15 a 1:20 |
| T7 (Blender) | 1:40 a 2:25 |
| T8 (input) | 3:00–3:55 |
| T9, T10 (Fresnel, environment mapping) | 2:25–3:00 |

## Edición y entrega
- Rótulos: se agregan en cualquier editor (DaVinci Resolve, Clipchamp, CapCut). Fuente grande, fondo oscuro semitransparente.
- Duración final entre 3:00 y 5:00; si se pasa de 5:00, recortar el tramo de la escena 1.
- Subir a YouTube como "no listado" o a Drive con enlace público, y poner el enlace en el reporte (español e inglés).
