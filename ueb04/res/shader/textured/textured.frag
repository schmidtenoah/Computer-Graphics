#version 430 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D u_textures[6];

void main() {
    int faceID = (gl_PrimitiveID / 2) % 6;
    fragColor = texture(u_textures[faceID], texCoords);
}