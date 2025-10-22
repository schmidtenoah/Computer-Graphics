#version 430 core

in vec2 fragCoord;
out vec4 fragColor;

uniform vec3 u_topColor = vec3(0.2, 0.3, 0.6);    // Dunkelblau oben
uniform vec3 u_bottomColor = vec3(0.6, 0.7, 0.9); // Hellblau unten

void main() {
    // Vertikaler Gradient: y=0 unten, y=1 oben
    vec3 color = mix(u_bottomColor, u_topColor, fragCoord.y);
    fragColor = vec4(color, 1.0);
}