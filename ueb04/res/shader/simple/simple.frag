#version 430 core

out vec4 fragColor;

uniform vec3 u_color;
uniform bool u_drawInstanced;

flat in int isLeader;
flat in int vertID;

void main() {
    vec3 color;

    if (u_drawInstanced && isLeader == 0) {
        color = vec3(1.0, 0.0, 0.0);
    } else {
        if (vertID == 0) color = vec3(1, 0, 0); 
        else if (vertID == 1) color = vec3(0, 1, 0); 
        else if (vertID == 2) color = vec3(0, 0, 1);
        else color = vec3(1, 0, 1); 
    }

    fragColor = vec4(color, 1.0);
}