#ifndef LOGIC_H
#define LOGIC_H

#include <fhwcg/fhwcg.h>
#include "input.h"

void logic_update(InputData *data, vec2 *ctrl, int n);

void logic_init();

void logic_skipLevel(InputData *data);

void logic_restartLevel(InputData *data);

void loadLevel(int idx, InputData *data);

#endif // LOGIC_H