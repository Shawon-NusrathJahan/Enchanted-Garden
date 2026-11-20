
#ifndef ANIMATION_H
#define ANIMATION_H

void initializeAnimation(double currentTime);
void updateAnimation(double currentTime);
void toggleAnimation();
void resetAnimation(double currentTime);

float getZoom();
float getParallax();

#endif
