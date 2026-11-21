// Enchanted Garden - cinematic mountain -> village -> garden animation
// C++17, OpenGL 2.1 (immediate mode), GLFW + GLEW
//
// Keys:   SPACE  pause / resume          R        restart
//         1..5   jump to a stage         <- / ->  skip -2 s / +2 s
//         O      view culling on / off   ESC      quit
//
// Optional: EnchantedGarden --shot <seconds> <file.ppm>   saves one frame and exits

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "animation.h"
#include "garden.h"
#include "gfx.h"
#include "mountain.h"

using namespace std;

static const int   START_W = 1280;
static const int   START_H = 720;
static const float ASPECT  = 16.0f / 9.0f;

static GLuint captureTex = 0;

// ---------------------------------------------------------------------------
// Village -> garden dissolve: copy the finished village frame into a texture,
// draw the garden, then lay the village texture over it with fading alpha.
// ---------------------------------------------------------------------------
static void captureFrame(int vx, int vy, int vw, int vh)
{
    if (!captureTex) glGenTextures(1, &captureTex);
    glBindTexture(GL_TEXTURE_2D, captureTex);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, vx, vy, vw, vh, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

static void drawCapture(float alpha)
{
    const float W = gfx::WORLD_W;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, captureTex);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-W, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(W, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(W, 1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-W, 1.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

// Warm bloom of light that peaks in the middle of the dissolve. It hides the
// moment where giant village walls and the garden overlap and gives the
// transition an "enchanted" feel.
static void drawBloom(float blend)
{
    const float a = 0.92f * std::pow(std::sin(gfx::PI * blend), 1.5f);
    if (a <= 0.001f) return;
    const float W = gfx::WORLD_W;
    glColor4f(1.0f, 0.97f, 0.86f, 0.55f * a);
    gfx::rect(-W, -1.0f, W, 1.0f);
    gfx::glow(0.0f, 0.0f, 3.0f, gfx::rgb(1.0f, 0.97f, 0.86f, a), 48);
}

// ---------------------------------------------------------------------------
// One frame
// ---------------------------------------------------------------------------
static void renderFrame(GLFWwindow* window)
{
    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    if (fbw <= 0 || fbh <= 0) return;                           // minimised

    // keep a fixed 16:9 picture, black bars if the window has another shape
    int vw = fbw, vh = (int)(fbw / ASPECT);
    if (vh > fbh) { vh = fbh; vw = (int)(fbh * ASPECT); }
    const int vx = (fbw - vw) / 2, vy = (fbh - vh) / 2;

    glDisable(GL_SCISSOR_TEST);
    glViewport(0, 0, fbw, fbh);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(vx, vy, vw, vh);
    glScissor(vx, vy, vw, vh);
    glEnable(GL_SCISSOR_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-gfx::WORLD_W, gfx::WORLD_W, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gfx::statDrawn  = 0;
    gfx::statCulled = 0;

    const AnimState& s = getAnimState();

    if (s.gardenBlend < 1.0f) drawMountainScene(s);             // stages 1-4 (village side)

    if (s.gardenBlend > 0.0f)                                   // stages 4-6 (garden side)
    {
        const bool fading = s.gardenBlend < 1.0f;
        if (fading)
        {
            captureFrame(vx, vy, vw, vh);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        drawGarden(s);
        if (fading)
        {
            drawCapture(1.0f - s.gardenBlend);
            drawBloom(s.gardenBlend);
        }
    }
}

static void savePPM(const char* path, int w, int h)
{
    vector<unsigned char> px((size_t)w * h * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, px.data());
    FILE* f = fopen(path, "wb");
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", w, h);
    for (int y = h - 1; y >= 0; y--) fwrite(&px[(size_t)y * w * 3], 1, (size_t)w * 3, f);
    fclose(f);
}

static void keyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (action != GLFW_PRESS) return;
    const double now = glfwGetTime();

    switch (key)
    {
    case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
    case GLFW_KEY_SPACE:  toggleAnimation(); break;
    case GLFW_KEY_R:      resetAnimation(now); break;
    case GLFW_KEY_1:      jumpToPhase(PHASE_OPENING, now); break;
    case GLFW_KEY_2:      jumpToPhase(PHASE_REVEAL, now); break;
    case GLFW_KEY_3:      jumpToPhase(PHASE_JOURNEY, now); break;
    case GLFW_KEY_4:      jumpToPhase(PHASE_GARDEN_REVEAL, now); break;
    case GLFW_KEY_5:      jumpToPhase(PHASE_LIVING_GARDEN, now); break;
    case GLFW_KEY_LEFT:   seekAnimation(-2.0, now); break;
    case GLFW_KEY_RIGHT:  seekAnimation(2.0, now); break;
    case GLFW_KEY_O:      gfx::cullingEnabled = !gfx::cullingEnabled; break;
    default: break;
    }
}

int main(int argc, char** argv)
{
    // optional single-frame capture mode
    bool        shotMode = false;
    double      shotTime = 0.0;
    const char* shotFile = "shot.ppm";
    if (argc >= 4 && string(argv[1]) == "--shot")
    {
        shotMode = true;
        shotTime = atof(argv[2]);
        shotFile = argv[3];
    }

    if (!glfwInit())
    {
        cout << "GLFW initialization failed!" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_SAMPLES, 4);                            // smooth edges (MSAA)

    GLFWwindow* window = glfwCreateWindow(START_W, START_H, "Enchanted Garden", NULL, NULL);
    if (window == NULL)
    {
        cout << "Window creation failed!" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);                                        // vsync
    glfwSetKeyCallback(window, keyCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        cout << "GLEW initialization failed!" << endl;
        glfwTerminate();
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    initMountain();
    initGarden();
    initializeAnimation(glfwGetTime());

    if (shotMode)
    {
        seekAnimation(shotTime, glfwGetTime());
        // draw twice so smoke / loop clocks are settled, then save the back buffer
        renderFrame(window);
        int w = 0, h = 0;
        glfwGetFramebufferSize(window, &w, &h);
        savePPM(shotFile, w, h);
        glfwTerminate();
        return 0;
    }

    double fpsTimer = glfwGetTime();
    int    frames   = 0;

    while (!glfwWindowShouldClose(window))
    {
        updateAnimation(glfwGetTime());
        renderFrame(window);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // window title: stage, time, fps, culling statistics
        frames++;
        const double now = glfwGetTime();
        if (now - fpsTimer >= 0.5)
        {
            char title[256];
            const AnimState& s = getAnimState();
            snprintf(title, sizeof(title),
                     "Enchanted Garden | stage %s%s | t=%.1fs | %.0f FPS | culling %s (drawn %d, skipped %d)",
                     phaseName(s.phase), s.paused ? " [PAUSED]" : "", s.time, frames / (now - fpsTimer),
                     gfx::cullingEnabled ? "ON" : "OFF", gfx::statDrawn, gfx::statCulled);
            glfwSetWindowTitle(window, title);
            fpsTimer = now;
            frames   = 0;
        }
    }

    glfwTerminate();
    return 0;
}
