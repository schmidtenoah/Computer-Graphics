#ifndef SHADER_H
#define SHADER_H

#include <fhwcg/fhwcg.h>

void shader_cleanup(void);

void shader_load(void);

void shader_setMVP(void);

void shader_setColor(vec3 color);

void shader_setNormals(void);

#endif // SHADER_H