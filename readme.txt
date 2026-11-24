T68 SCI-FI TRIBUTE
Proyecto final de Teoria - Computacion Grafica e Interaccion Humano-Computadora, 2027-1
Facultad de Ingenieria, UNAM. Profesor: Ing. Jose Ramon Perez Athie.
Tema: "Mexico y su cultura" - Plaza de las Tres Culturas (Tlatelolco), 2 de octubre de 1968, y la ofrenda.

REQUISITOS MINIMOS
- Windows 10 u 11 de 64 bits.
- Tarjeta de video con OpenGL 3.3 o superior (controladores actualizados).
- 2 GB de RAM libres y unos 300 MB de disco.
- El programa tarda alrededor de 10 segundos en abrir (carga la plaza completa).

INSTALACION
1. Ejecuta el instalador (.exe) y sigue los pasos.
2. Abre "T68 Sci-Fi Tribute" desde el menu Inicio o el acceso directo del escritorio.
3. No muevas ni borres las carpetas Shader, Models y UI que se instalan junto al ejecutable.
Para desinstalar: Configuracion de Windows > Aplicaciones, o el desinstalador del menu Inicio.

COMO SE USA (RELATO INTERACTIVO)
El programa abre en modo relato: una historia por escenas con texto en pantalla.
- ENTER o ESPACIO ........ avanzar el texto / continuar
- FLECHAS IZQ / DER ....... acciones de las escenas interactivas (esquives, avances)
- RePag / AvPag (o [ y ]) . escena anterior / siguiente
- ESC ..................... salir

MODO LIBRE Y CAMARA
- F1 ...................... camara libre (WASD o flechas + mouse)
- F2 ...................... modo banco de pruebas (personajes sueltos para probar las animaciones)
- F3 ...................... modo edicion de actores (solo desarrollo)

ANIMACIONES POR TECLADO (en modo banco de pruebas, F2)
- 1 .. Sierras de la MAC girando ............ basica
- 2 .. Barrido y disparos de los SENTINEL .... procedural
- 3 .. Zigzag esquivando de CUAU-68 .......... procedural
- 4 .. Caminata por partes de los robots ..... jerarquia, keyframes en codigo
- 5 .. Obreros con martillo (ESPACIO golpea) .. keyframes en codigo
- 6 .. Animacion de huesos de CUAU-68 (Mixamo) .. huesos
- 7 .. Animacion de huesos Victory de CUAU-68 (Mixamo) .. huesos
- 8 .. Caida del SENTINEL central .............. procedural
Tambien funcionan las letras N, B, V, G, F (equivalen a 1, 2, 3, 4, 5) y K (cambia de clip de huesos).
En la escena final del relato (ESPACIO en la escena 8), CUAU celebra con la animacion Victory.
Siempre activas (animadas en Blender): papel picado ondeando y petalos cayendo en la ofrenda.
- M .. Recorrido de camara animado en Blender por la iglesia (T7)

LUCES EN TIEMPO REAL
- L .. Linterna: un reflector que sigue a la camara
- H .. Luces del casco y visor de CUAU-68 (con luz puntual)
- T .. Alternar dia / noche

ILUMINACION Y AMBIENTE
- E .. Activa / desactiva Fresnel y environment mapping (reflejo y refraccion del cielo en el visor y el metal). Apagado = solo Phong.
- El cielo (skybox) es un cubemap que tambien se refleja en los materiales.

CREDITOS
- Modelos: robots CUAU-68, SENTINEL-MX, railgun y MAC: Santiago Velasco. Plaza: Victor. Ofrenda: Erick.
- Animacion de huesos: Mixamo (Adobe), aplicada al rig de CUAU-68.
- Texturas de la plaza: Poly Haven (rock_wall_08, concrete_wall_006, plastered_wall_02, grass_ground).
- Imagenes del papel picado y la catrina: ver las referencias del reporte.
