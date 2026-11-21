#ifndef VILLAGE_H
#define VILLAGE_H

void initVillage();

// Whole village (houses, towers, trees, fences). Coordinates are valley
// coordinates; the caller has already applied the layer/camera transform.
void drawVillage(double time);

// One cottage. x,y = centre of the ground line, w = width, style = 0..3.
// Reused by the garden so the village house "becomes" the garden cottage.
void drawHouse(float x, float y, float w, int style, bool smoke, double time);

#endif
