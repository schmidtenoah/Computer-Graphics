/**
 * 
 */

#ifndef INPUT_H
#define INPUT_H

#include <fhwcg/fhwcg.h>
#define MAX_STARS 40

typedef void (*CurveEvalFn)(vec2* ctrl, int numPoints, float t, vec2 dest, bool *updateCoeffs);

typedef struct {
    bool isFullscreen;
    bool showWireframe;
    bool showHelp;
    bool showMenu;
    float deltaTime;
    bool paused;
    struct {
        int button;
        int action;
        float xPos, yPos;
    } mouse;

    struct {
        CurveEvalFn curveEval;
        float width;
        float resolution;
        bool drawPolygon;
        bool drawConvexHull;
        bool showNormals;
        int buttonCount;
        bool resolutionChanged;
        bool buttonsChanged;
    } curve;

    struct {
        bool isFlying;
        bool showColliders;
        struct {
            vec2 position;
            float rotation;
            vec2 vertices[3];
            float colliderRadius;
            float defaultSpeed;
        } airplane;
        struct {
            vec2 *pos;
            int n;
            float colliderRadius;
        } stars, clouds;
        bool collected[MAX_STARS];
        int currentLevel;
    } game;

} InputData;

void input_init(ProgContext ctx);

InputData* getInputData(void);

void input_registerCallbacks(ProgContext ctx);

#endif // INPUT_H