#version 430 core

/**
 * @file
 * 
 * @brief Ein Shader zum Anzeigen von Normalen von 3D Modellen aus Dreiecken.
 *
 * Siehe Normals Demo für ein Beispiel.
 *
 */

// Kleiner Workaround, da Doxygen kein GLSL unterstützt und es als C code geparsed wird
#ifdef _DOC
   #define GLSL_INPUT(loc, type, name, comment) /** comment (location = loc)*/ \
in type name;
#else
   #define GLSL_INPUT(loc, type, name, comment) layout(location = loc) in type name;

    out VS_OUT {
        vec3 pos;
        vec3 normal;
    } vs_out;
#endif

GLSL_INPUT(0, vec3, pos, Vertexposition im Model Space)
GLSL_INPUT(1, vec3, normal, Vertexnormale im Model Space)

/** Die kombinierte Model-View-Matrix*/
uniform mat4 u_modelViewMatrix;

/** Die Normal Matrix */
uniform mat4 u_normalMatrix;

void main() {
    vs_out.pos = vec3(u_modelViewMatrix * vec4(pos, 1));
    vs_out.normal = vec3(u_normalMatrix * vec4(normal, 0));
}
