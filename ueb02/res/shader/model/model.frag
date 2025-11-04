#version 430

uniform vec3 u_color = vec3(1, 1, 1); 

out vec4 fragColor;

#define HEIGHT_LOWEST -5.0
#define HEIGHT_VERY_LOW -1.0
#define HEIGHT_LOW -0.25
#define HEIGHT_MEDIUM 0.001
#define HEIGHT_HIGH 0.25
#define HEIGHT_VERY_HIGH 0.6
#define HEIGHT_HIGHEST 2.0

vec3 COLOR_LOWEST = vec3(0.0, 0.0, 1.0);
vec3 COLOR_VERY_LOW = vec3(0.1, 0.0, 0.7);
vec3 COLOR_LOW = vec3(0.0, 0.0, 0.5);
vec3 COLOR_MEDIUM = vec3(0.0, 0.5, 0.0);
vec3 COLOR_HIGH = vec3(0.5, 0.5, 0.0);
vec3 COLOR_VERY_HIGH = vec3(0.5, 0.2, 0.3);
vec3 COLOR_HIGHEST = vec3(0.5, 0.1, 0.1);

in VS_OUT {
    vec2 TexCoords;
    vec3 PositionWS;
} fs_in;

void main(void) {
    float height = fs_in.PositionWS.y;

    vec3 color;
    if (height <= HEIGHT_LOWEST) {
        color = COLOR_LOWEST;
    } else if (height <= HEIGHT_VERY_LOW) {
        color = COLOR_VERY_LOW;
    } else if (height <= HEIGHT_LOW) {
        color = COLOR_LOW;
    } else if (height <= HEIGHT_MEDIUM) {
        color = COLOR_MEDIUM;
    } else if (height <= HEIGHT_HIGH) {
        color = COLOR_HIGH;
    } else if (height <= HEIGHT_VERY_HIGH) {
        color = COLOR_VERY_HIGH;
    } else {
        color = COLOR_HIGHEST;
    }

    fragColor = vec4(color, 1.0);
}
