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
 * @param drawInstanced Whether instanced drawing is enabled.
 */
void shader_setSimpleMVP(bool drawInstanced);

/**
 * Sets instance-specific data for the simple shader.
 * @param scale Local scale vector for instances.
 * @param leaderIdx Index of the leader particle (-1 if none).
 * @param hardColor If the color should have a noticable seam.
 */
void shader_setSimpleInstanceData(vec3 scale, int leaderIdx, bool hardColor);

/**
 * Sets visualization data for particle vectors.
 * @param scale Local scale vector for particle visualization.
 */
void shader_setParticleVisData(vec3 scale);

/**
 * Sets drop shadow rendering parameters.
 * @param scale Local scale vector for instances.
 * @param leaderIdx Index of the leader particle (-1 if none).
 * @param drawInstanced Whether instanced drawing is enabled.
 * @param groundHeight Height of the ground plane for shadow projection.
 */
void shader_setDropShadowData(vec3 scale, int leaderIdx, bool drawInstanced, float groundHeight);

/**
 * Retrieves the Shader for drawing textured models.
 * @return Pointer to the texture shader.
 */
Shader* shader_getTextureShader(void);

#endif // SHADER_H
