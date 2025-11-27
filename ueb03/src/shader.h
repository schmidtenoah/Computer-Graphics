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
void shader_setMVP(mat4 *viewMat, mat4 *modelviewMat, bool useBallMat);

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
 * Sets the current Stack MVP-Matrix for the Simple-Shader.
 */
void shader_setSimpleMVP(void);

/**
 * Sets which texture to use for the Model-Shader.
 * @param textureId The id to use from now on.
 * @param useTexture If the texture should be used.
 */
void shader_setTexture(GLuint textureId, bool useTexture);

/**
 * Sets the camera position for the Model-Shader.
 * @param camPosWS The camera world position.
 */
void shader_setCamPos(vec3 camPosWS);

/**
 * Sets all point light attributes for the Model-Shader.
 * @param color The color of the light.
 * @param posWS The camera position in world space.
 * @param falloff The attenuation falloff values.
 * @param enabled If the light is on or off.
 * @param ambientFactor How much the lights ambient takes effect. 
 */
void shader_setPointLight(vec3 color, vec3 posWS, vec3 falloff, bool enabled, float ambientFactor);

#endif // SHADER_H
