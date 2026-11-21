#include "animation.h"
#include "gfx.h"

#include <cmath>

using gfx::clampf;
using gfx::smoothstep;

static double    elapsed  = 0.0;
static double    lastTime = 0.0;
static bool      playing  = true;
static AnimState state;

static Phase phaseAt(double t)
{
    if (t < timeline::REVEAL_START)  return PHASE_OPENING;
    if (t < timeline::JOURNEY_START) return PHASE_REVEAL;
    if (t < timeline::GARDEN_START)  return PHASE_JOURNEY;
    if (t < timeline::LIVING_START)  return PHASE_GARDEN_REVEAL;
    return PHASE_LIVING_GARDEN;
}

static double phaseStart(Phase p)
{
    switch (p)
    {
    case PHASE_OPENING:       return timeline::OPENING_START;
    case PHASE_REVEAL:        return timeline::REVEAL_START;
    case PHASE_JOURNEY:       return timeline::JOURNEY_START;
    case PHASE_GARDEN_REVEAL: return timeline::GARDEN_START;
    default:                  return timeline::LIVING_START;
    }
}

// Turn elapsed time into the animation state.
//   u = clamp((t - t0) / (t1 - t0), 0, 1)       E(u) = 3u^2 - 2u^3
static void recompute()
{
    using namespace timeline;
    const float t = (float)elapsed;

    state.time   = elapsed;
    state.phase  = phaseAt(elapsed);
    state.paused = !playing;

    // Stage 2: the foreground mountain slides apart.
    state.open = smoothstep((t - (float)REVEAL_START) / (float)(JOURNEY_START - REVEAL_START));

    // Parallax: the camera drifts slowly sideways over the whole panorama and
    // re-centres while the journey starts. Layers shift by (-camX * depth).
    const float driftBase = gfx::lerpf(-0.12f, 0.12f, smoothstep(t / 11.0f));
    const float driftFade = 1.0f - smoothstep((t - 10.0f) / 4.0f);
    state.camX = driftBase * driftFade;

    // Stage 3: forward journey, scale grows exponentially (constant zoom feel).
    state.travel = smoothstep((t - (float)JOURNEY_START) / (float)(LIVING_START - JOURNEY_START));
    // A slow forward glide (1.0 -> 1.3) runs through the whole panorama, then the
    // journey takes over, so the camera never stops moving.
    state.zoom   = (1.0f + 0.12f * smoothstep(t / 11.0f)) * std::pow(ZOOM_MAX, state.travel);

    state.shake = 0.0f;

    // Stage 4: village -> garden dissolve. Garden animation clock starts here.
    state.gardenBlend = smoothstep((t - (float)GARDEN_START) / (float)(LIVING_START - GARDEN_START));
    state.gardenTime  = elapsed > GARDEN_START ? elapsed - GARDEN_START : 0.0;
}

void initializeAnimation(double currentTime)
{
    elapsed  = 0.0;
    lastTime = currentTime;
    playing  = true;
    recompute();
}

void updateAnimation(double currentTime)
{
    double dt = currentTime - lastTime;
    lastTime  = currentTime;
    if (dt > 0.1) dt = 0.1;            // avoid jumps after window drag / breakpoints
    if (dt < 0.0) dt = 0.0;
    if (playing) elapsed += dt;
    recompute();
}

void toggleAnimation() { playing = !playing; recompute(); }

void resetAnimation(double currentTime) { initializeAnimation(currentTime); }

void jumpToPhase(Phase phase, double currentTime)
{
    elapsed  = phaseStart(phase);
    lastTime = currentTime;
    recompute();
}

void seekAnimation(double deltaSeconds, double currentTime)
{
    elapsed += deltaSeconds;
    if (elapsed < 0.0) elapsed = 0.0;
    lastTime = currentTime;
    recompute();
}

const AnimState& getAnimState() { return state; }

const char* phaseName(Phase phase)
{
    switch (phase)
    {
    case PHASE_OPENING:       return "1 Mountain Panorama";
    case PHASE_REVEAL:        return "2 Village Reveal";
    case PHASE_JOURNEY:       return "3 Forward Journey";
    case PHASE_GARDEN_REVEAL: return "4 Garden Reveal";
    default:                  return "5 Living Garden (loop)";
    }
}
