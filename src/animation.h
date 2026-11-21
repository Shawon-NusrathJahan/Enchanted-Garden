// animation.h  -  central, time based animation controller
//
// One clock drives everything. The controller turns elapsed seconds into a
// small AnimState that the scenes read (they never keep their own timers).

#ifndef ANIMATION_H
#define ANIMATION_H

enum Phase
{
    PHASE_OPENING = 0,      // mountain panorama, slow parallax drift (no village yet)
    PHASE_REVEAL,           // near ridges slide away, the village shows up
    PHASE_JOURNEY,          // zoom toward the village
    PHASE_GARDEN_REVEAL,    // village dissolves into the garden
    PHASE_LIVING_GARDEN     // looping garden (runs forever)
};

// ---- Timeline (seconds). Change these numbers to retime the whole film. ----
namespace timeline
{
constexpr double OPENING_START = 0.0;
constexpr double REVEAL_START  = 6.0;    // near ridges start sliding away
constexpr double JOURNEY_START = 10.0;   // village fully revealed, zoom begins
constexpr double GARDEN_START  = 18.5;   // garden starts fading in + animating
constexpr double LIVING_START  = 21.5;   // garden fully visible, loops forever
constexpr float  ZOOM_MAX      = 9.0f;   // final zoom factor on the village
}

struct AnimState
{
    double time;          // seconds since start (frozen while paused)
    Phase  phase;
    float  open;          // 0 = near ridges hide the village, 1 = slid away (smoothstep)
    float  camX;          // sideways camera drift used for parallax
    float  zoom;          // forward-journey zoom factor (1 .. ZOOM_MAX)
    float  travel;        // 0..1 progress of the forward journey
    float  shake;         // unused (kept at 0)
    float  gardenBlend;   // 0 = village only .. 1 = garden only
    double gardenTime;    // seconds since the garden started animating
    bool   paused;
};

void initializeAnimation(double currentTime);
void updateAnimation(double currentTime);
void toggleAnimation();
void resetAnimation(double currentTime);
void jumpToPhase(Phase phase, double currentTime);
void seekAnimation(double deltaSeconds, double currentTime);   // skip forward / back

const AnimState& getAnimState();
const char*      phaseName(Phase phase);

#endif
