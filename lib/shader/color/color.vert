#version 430 core

/**
 * @file
 *
 * @brief Ein einfacher 3D Shader, der Objekte in einer festen Farbe anzeigt.
 *
 * Dieser Shader wird zum Beispiel in der Stack Demo verwendet und kann auch für das Achsen Widget des GUI genutzt werden.
 *
 */

// Kleiner Workaround, da Doxygen kein GLSL unterstützt und es als C code geparsed wird
#ifdef _DOC
   #define GLSL_INPUT(loc, type, name, comment) /** comment (location = loc)*/ \
in type name;
#else
   #define GLSL_INPUT(loc, type, name, comment) layout(location = loc) in type name;
#endif

GLSL_INPUT(0, vec3, position, Vertexposition im Model Space)

/** Die kombinierte Model-View-Projection-Matrix*/
uniform mat4 u_mvpMatrix;

void main() {
   gl_Position = u_mvpMatrix * vec4(position, 1);
}
