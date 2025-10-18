#version 430 core

layout(points) in;
layout(line_strip, max_vertices = 2) out;

in VS_OUT {
    vec3 pos;
    vec3 normal;
} geo_in[];

uniform mat4 u_projMatrix;
uniform float u_normalLength = 0.1;

void main() {
    vec3 p = geo_in[0].pos;
    vec3 n = normalize(geo_in[0].normal);

    // start point
    gl_Position = u_projMatrix * vec4(p, 1.0);
    EmitVertex();

    // end point (normal tip)
    gl_Position = u_projMatrix * vec4(p + n * u_normalLength, 1.0);
    EmitVertex();

    EndPrimitive();
}
