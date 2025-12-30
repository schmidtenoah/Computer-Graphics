#version 430 core

layout(points) in;
layout(line_strip, max_vertices = 4) out;

in VS_OUT {
    vec3 worldPos;
    vec3 acceleration;
    vec3 up;
} gs_in[];

uniform mat4 u_mvpMatrix;
out vec3 gsColor;

void main() {
    vec3 p = gs_in[0].worldPos;

    // Acceleration line
    float scale = 0.2;
    gsColor = vec3(1, 0, 0);
    gl_Position = u_mvpMatrix * vec4(p, 1.0);
    EmitVertex();
    gl_Position = u_mvpMatrix * vec4(p + gs_in[0].acceleration * scale, 1.0);
    EmitVertex();
    EndPrimitive();

    // Up line
    gsColor = vec3(0, 0, 1);
    gl_Position = u_mvpMatrix * vec4(p, 1.0);
    EmitVertex();
    gl_Position = u_mvpMatrix * vec4(p + gs_in[0].up, 1.0);
    EmitVertex();
    EndPrimitive();
}
