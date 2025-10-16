#version 430 core

/**
 * @file
 * 
 * @brief Ein Shader zum Anzeigen von Normalen von 3D Modellen aus Dreiecken.
 *
 * Siehe Normals Demo für ein Beispiel.
 *
 */

/** Ausgabefarbe */
out vec4 FragColor;

/** Farbe der Linien */
uniform vec3 u_color = vec3(1, 0, 0);

void main() {
    FragColor = vec4(u_color, 1.0);
}
