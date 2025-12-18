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
    vec3 NormalVS;
    vec3 PositionVS;
} vs_out;

/**
 * Model Vertex Shader Main.
 * Calculates the model and view space position of the vertex and
 * transforms the normal in the view space.
 */
void main(void) {
    mat4 viewInverse = inverse(u_viewMatrix);
    mat4 model = viewInverse * u_modelviewMatrix;
    vs_out.PositionWS = vec3(model * vec4(pos, 1.0));
    vs_out.TexCoords = tex;
    vs_out.PositionVS = vec3(u_modelviewMatrix * vec4(pos, 1.0));

    mat3 normalMatrix = transpose(inverse(mat3(u_modelviewMatrix)));
    vs_out.NormalVS = normalize(normalMatrix * norm);

    gl_Position = u_mvpMatrix * vec4(pos, 1);
}
