#version 430 core

/**
 * @file
 *
 * @brief Ein einfacher 3D Shader, der Objekte in einer festen Farbe anzeigt.
 *
 * Dieser Shader wird zum Beispiel in der Stack Demo verwendet und kann auch für das Achsen Widget des GUI genutzt werden.
 *
 */

/** Ausgabefarbe */
out vec4 fragColor;

/** Farbe, in der das Objekt angezeigt wird */
uniform vec3 u_color = vec3(0, 1, 1);

void main() {
   fragColor = vec4(u_color, 1);
}
