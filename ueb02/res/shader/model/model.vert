#version 430

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;

uniform mat4 u_mvpMatrix;
uniform mat4 u_modelviewMatrix;
uniform mat4 u_viewMatrix;

out VS_OUT {
    vec2 TexCoords;
    vec3 PositionWS;
} vs_out;

void main(void) {
    mat4 viewInverse = inverse(u_viewMatrix);
    mat4 model = viewInverse * u_modelviewMatrix;
    vs_out.PositionWS = vec3(model * vec4(pos, 1.0));
    vs_out.TexCoords = tex;

    gl_Position = u_mvpMatrix * vec4(pos, 1);
}
