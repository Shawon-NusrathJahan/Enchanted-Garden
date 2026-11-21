#ifndef GARDEN_H
#define GARDEN_H

#include "animation.h"

// Every periodic motion in the garden uses a whole number of cycles per loop,
// so at t = GARDEN_LOOP_SECONDS everything is exactly back at its start pose.
constexpr double GARDEN_LOOP_SECONDS = 60.0;

void initGarden();
void drawGarden(const AnimState& s);

#endif
