
#include "animation.h"

static double elapsed = 0.0;
static double lastTime = 0.0;

static bool playing = true;

void initializeAnimation(double currentTime)
{
    elapsed = 0.0;
    lastTime = currentTime;
    playing = true;
}

void updateAnimation(double currentTime)
{
    double deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    if (deltaTime > 0.1)
        deltaTime = 0.1;

    if (playing && elapsed < 18.0)
    {
        elapsed += deltaTime;

        if (elapsed > 18.0)
            elapsed = 18.0;
    }
}

void toggleAnimation()
{
    playing = !playing;
}

void resetAnimation(double currentTime)
{
    elapsed = 0.0;
    lastTime = currentTime;
    playing = true;
}

float getZoom()
{
    float progress = (float)(elapsed / 18.0);

    return 1.0f + 1.1f * progress;
}

float getParallax()
{
    return (float)(elapsed / 18.0);
}
