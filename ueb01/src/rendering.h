/**
 * 
 */

#ifndef RENDERING_H
#define RENDERING_H

#include <fhwcg/fhwcg.h>

#define CURVE_MAX_VERTICES 10000
#define EPSILON 1e-6f

#define BUTTON_COUNT 20

typedef struct {
    vec2 center;
    float r;
} Circle;

typedef struct {
    ivec2 screenRes;
    float aspect;
    float left, right, top, bottom;
} RenderingData;

//PUBLIC

void rendering_init(void);

void initButtons(int btnCnt);

void rendering_draw(void);

void rendering_cleanup(void);

void rendering_resize(int width, int height, int btnCnt);

#endif // RENDERING_H