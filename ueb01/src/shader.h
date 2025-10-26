/**
 * @file shader.h
 * @brief Shader program management and uniforms.
 *
 * Has functions to load, activate and configure shader programs.
 * Manages simple color shader, gradient shader and normal visualization shader.
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef SHADER_H
#define SHADER_H

#include <fhwcg/fhwcg.h>

/**
 * Deletes all shaders and frees GPU memory.
 */
void shader_cleanup(void);

/**
 * Loads and compiles all shader programs.
 * Creates simple shader, gradient shader and normal shader.
 * If compilation succeeds, replaces existing shaders.
 */
void shader_load(void);

/**
 * Activates simple shader and sets MVP matrix uniform.
 * Gets the combined MVP matrix from scene and uploads it
 * to shader for vertex transformation.
 */
void shader_setMVP(void);

/**
 * Sets the color uniform in the simple shader.
 *
 * @param color RGB color value used
 */
void shader_setColor(vec3 color);

/**
 * Activates the normal shader and sets uniforms
 * Prepares matrices for geometry shader which generates visible normal vectors.
 */
void shader_setNormals(void);

/**
 * Activates gradient shader and sets its color uniforms.
 * Configures top and bottom colors for vertical gradient rendering.
 */
void shader_renderGradient(void);

#endif // SHADER_H