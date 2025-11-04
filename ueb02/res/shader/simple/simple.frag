#version 430

uniform vec3 u_color = vec3(1, 1, 1); 

out vec4 fragColor;

void main(void) {
    fragColor = vec4(u_color, 1.0);
}