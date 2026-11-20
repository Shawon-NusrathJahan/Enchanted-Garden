#include "mountain.h"
#include <GL/glew.h>
#include <cmath>
#include <algorithm>

namespace
{
    const float PI = 3.1415926535f;

    void color(float r, float g, float b)
    {
        glColor3f(r, g, b);
    }

  void polygon(const float* xy, int count)
{
    glBegin(GL_POLYGON);

    for (int i = 0; i < count; ++i)
    {
        glVertex2f(xy[i * 2], xy[i * 2 + 1]);
    }

    glEnd();
}

    void quad(float x1, float y1, float x2, float y2,
              float x3, float y3, float x4, float y4)
    {
        glBegin(GL_QUADS);
        glVertex2f(x1, y1); glVertex2f(x2, y2);
        glVertex2f(x3, y3); glVertex2f(x4, y4);
        glEnd();
    }

    void triangle(float x1, float y1, float x2, float y2,
                  float x3, float y3)
    {
        glBegin(GL_TRIANGLES);
        glVertex2f(x1, y1); glVertex2f(x2, y2); glVertex2f(x3, y3);
        glEnd();
    }

    void disk(float cx, float cy, float rx, float ry, int segments = 36)
    {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= segments; ++i)
        {
            float a = 2.0f * PI * i / segments;
            glVertex2f(cx + std::cos(a) * rx, cy + std::sin(a) * ry);
        }
        glEnd();
    }

    void line(float x1, float y1, float x2, float y2, float width)
    {
        glLineWidth(width);
        glBegin(GL_LINES);
        glVertex2f(x1, y1); glVertex2f(x2, y2);
        glEnd();
        glLineWidth(1.0f);
    }

    // Quadratic Bezier stroke, useful for the river and winding road.
    void curve(float x0, float y0, float x1, float y1,
               float x2, float y2, float width)
    {
        glLineWidth(width);
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i <= 70; ++i)
        {
            float t = i / 70.0f;
            float u = 1.0f - t;
            float x = u*u*x0 + 2*u*t*x1 + t*t*x2;
            float y = u*u*y0 + 2*u*t*y1 + t*t*y2;
            glVertex2f(x, y);
        }
        glEnd();
        glLineWidth(1.0f);
    }

    void cloud(float x, float y, float s)
    {
        color(0.98f, 0.88f, 0.79f);
        disk(x, y, s * 1.10f, s * 0.42f);
        disk(x - s * 0.48f, y + s * 0.12f, s * 0.52f, s * 0.48f);
        disk(x, y + s * 0.24f, s * 0.58f, s * 0.62f);
        disk(x + s * 0.48f, y + s * 0.12f, s * 0.55f, s * 0.48f);

        color(1.0f, 0.96f, 0.88f);
        disk(x - s * 0.12f, y + s * 0.27f, s * 0.36f, s * 0.34f);
        disk(x + s * 0.36f, y + s * 0.19f, s * 0.28f, s * 0.25f);
    }

    void skyAndClouds()
    {
        // Vertical sky bands create a soft painted-sky impression.
        color(0.22f, 0.52f, 0.79f);
        quad(-1.4f, -0.1f, 1.4f, -0.1f, 1.4f, 1.1f, -1.4f, 1.1f);

        color(0.40f, 0.70f, 0.88f);
        quad(-1.4f, 0.22f, 1.4f, 0.22f, 1.4f, 0.58f, -1.4f, 0.58f);

        color(0.94f, 0.73f, 0.65f);
        quad(-1.4f, 0.18f, 1.4f, 0.18f, 1.4f, 0.24f, -1.4f, 0.24f);

        color(1.0f, 0.83f, 0.64f);
        disk(0.68f, 0.73f, 0.105f, 0.105f);

        cloud(-0.78f, 0.77f, 0.105f);
        cloud(-0.30f, 0.86f, 0.075f);
        cloud(0.08f, 0.72f, 0.065f);
        cloud(0.88f, 0.88f, 0.10f);

        // Small distant wisps.
        color(0.91f, 0.90f, 0.86f);
        curve(-1.0f, 0.57f, -0.72f, 0.60f, -0.48f, 0.57f, 2.0f);
        curve(0.18f, 0.61f, 0.42f, 0.64f, 0.64f, 0.61f, 2.0f);
    }

    void distantMountains()
    {
        // Far blue range.
        color(0.25f, 0.47f, 0.68f);
        const float farRange[] = {
            -1.25f,-0.05f, -1.08f,0.34f, -0.91f,0.20f,
            -0.70f,0.48f, -0.49f,0.22f, -0.25f,0.55f,
            -0.03f,0.23f, 0.20f,0.48f, 0.43f,0.19f,
            0.67f,0.52f, 0.88f,0.25f, 1.08f,0.47f,
            1.25f,0.10f, 1.25f,-0.30f, -1.25f,-0.30f
        };
        polygon(farRange, 15);

        // Snow and light faces.
        color(0.73f, 0.84f, 0.89f);
        triangle(-0.88f,0.30f, -0.70f,0.48f, -0.61f,0.33f);
        triangle(-0.42f,0.38f, -0.25f,0.55f, -0.13f,0.37f);
        triangle(0.52f,0.37f, 0.67f,0.52f, 0.78f,0.37f);
        triangle(0.94f,0.36f, 1.08f,0.47f, 1.16f,0.32f);

        color(0.17f, 0.38f, 0.60f);
        const float secondRange[] = {
            -1.3f,-0.18f, -1.12f,0.18f, -0.91f,0.02f,
            -0.68f,0.30f, -0.43f,0.02f, -0.20f,0.27f,
            0.02f,0.00f, 0.26f,0.31f, 0.48f,0.02f,
            0.72f,0.27f, 0.96f,0.00f, 1.20f,0.21f,
            1.3f,-0.25f, 1.3f,-0.42f, -1.3f,-0.42f
        };
        polygon(secondRange, 15);

        // Cool atmospheric haze at the base of the mountains.
        color(0.52f, 0.72f, 0.79f);
        quad(-1.3f, -0.28f, 1.3f, -0.28f, 1.3f, -0.19f, -1.3f, -0.19f);
    }

    void middleRanges()
    {
        // Left blue-green ridge.
        color(0.10f, 0.34f, 0.42f);
        const float leftRidge[] = {
            -1.25f,-0.68f, -1.25f,0.13f, -1.05f,0.20f,
            -0.88f,0.06f, -0.68f,0.17f, -0.48f,-0.04f,
            -0.30f,-0.68f
        };
        polygon(leftRidge, 7);

        color(0.15f, 0.43f, 0.48f);
        triangle(-1.18f,0.07f, -1.05f,0.20f, -0.94f,0.04f);
        triangle(-0.87f,0.04f, -0.68f,0.17f, -0.56f,-0.02f);

        // Right blue-green ridge.
        color(0.08f, 0.30f, 0.39f);
        const float rightRidge[] = {
            1.25f,-0.68f, 1.25f,0.20f, 1.04f,0.27f,
            0.86f,0.08f, 0.66f,0.18f, 0.47f,-0.05f,
            0.28f,-0.68f
        };
        polygon(rightRidge, 7);

        color(0.18f, 0.45f, 0.47f);
        triangle(0.91f,0.09f, 1.04f,0.27f, 1.15f,0.10f);
        triangle(0.66f,0.18f, 0.77f,0.05f, 0.55f,0.00f);

        // Sunlit slopes.
        color(0.29f, 0.54f, 0.49f);
        triangle(-1.12f,-0.06f, -0.88f,0.06f, -0.62f,-0.24f);
        triangle(1.12f,-0.02f, 0.86f,0.08f, 0.61f,-0.26f);
    }

    void valleyFloor()
    {
        color(0.22f, 0.43f, 0.29f);
        const float valley[] = {
            -1.15f,-0.72f, -0.92f,-0.42f, -0.62f,-0.30f,
            -0.32f,-0.35f, -0.06f,-0.46f, 0.18f,-0.35f,
            0.48f,-0.39f, 0.76f,-0.52f, 1.15f,-0.75f,
            1.2f,-1.2f, -1.2f,-1.2f
        };
        polygon(valley, 11);

        // Lighter cultivated slopes.
        color(0.37f, 0.58f, 0.30f);
        const float meadow1[] = {
            -0.76f,-0.48f, -0.47f,-0.37f, -0.20f,-0.43f,
            -0.02f,-0.57f, -0.28f,-0.72f, -0.72f,-0.68f
        };
        polygon(meadow1, 6);

        color(0.48f, 0.64f, 0.31f);
        const float meadow2[] = {
            0.20f,-0.44f, 0.48f,-0.39f, 0.78f,-0.56f,
            0.55f,-0.76f, 0.18f,-0.67f, -0.02f,-0.57f
        };
        polygon(meadow2, 6);

        // Field/terrace contour strokes.
        color(0.60f, 0.68f, 0.34f);
        curve(-0.86f,-0.52f, -0.56f,-0.48f, -0.34f,-0.58f, 1.5f);
        curve(0.20f,-0.50f, 0.48f,-0.48f, 0.72f,-0.61f, 1.5f);
        curve(-0.72f,-0.63f, -0.43f,-0.61f, -0.22f,-0.69f, 1.2f);
    }

    void riverAndBanks()
    {
        // River bank shadow.
        color(0.10f, 0.34f, 0.39f);
        curve(0.15f,-0.34f, -0.30f,-0.50f, 0.04f,-0.68f, 11.0f);
        curve(0.04f,-0.68f, 0.42f,-0.81f, 0.06f,-1.12f, 11.0f);

        // Water.
        color(0.10f, 0.57f, 0.72f);
        curve(0.15f,-0.34f, -0.30f,-0.50f, 0.04f,-0.68f, 7.0f);
        curve(0.04f,-0.68f, 0.42f,-0.81f, 0.06f,-1.12f, 7.0f);

        // Bright reflections.
        color(0.57f, 0.84f, 0.83f);
        curve(0.11f,-0.42f, -0.08f,-0.48f, -0.04f,-0.54f, 1.5f);
        curve(0.09f,-0.73f, 0.25f,-0.80f, 0.20f,-0.86f, 1.2f);
    }

    void windingRoad()
    {
        // Road edge, then warm dirt surface.
        color(0.27f, 0.28f, 0.20f);
        curve(-0.88f,-0.88f, -0.52f,-0.72f, -0.30f,-0.78f, 9.0f);
        curve(-0.30f,-0.78f, -0.04f,-0.88f, 0.17f,-0.78f, 9.0f);
        curve(0.17f,-0.78f, 0.40f,-0.67f, 0.31f,-0.57f, 9.0f);
        curve(0.31f,-0.57f, 0.22f,-0.50f, 0.38f,-0.47f, 7.0f);

        color(0.77f, 0.62f, 0.39f);
        curve(-0.88f,-0.88f, -0.52f,-0.72f, -0.30f,-0.78f, 5.5f);
        curve(-0.30f,-0.78f, -0.04f,-0.88f, 0.17f,-0.78f, 5.5f);
        curve(0.17f,-0.78f, 0.40f,-0.67f, 0.31f,-0.57f, 5.5f);
        curve(0.31f,-0.57f, 0.22f,-0.50f, 0.38f,-0.47f, 4.0f);

        color(0.96f, 0.82f, 0.58f);
        curve(-0.72f,-0.83f, -0.50f,-0.75f, -0.37f,-0.77f, 1.0f);
    }

    void house(float x, float y, float s, float wallR, float wallG, float wallB)
    {
        glPushMatrix();
        glTranslatef(x, y, 0.0f);
        glScalef(s, s, 1.0f);

        // House shadow.
        color(0.12f, 0.25f, 0.20f);
        disk(0.015f, -0.012f, 0.105f, 0.025f);

        // Walls.
        color(wallR, wallG, wallB);
        quad(-0.075f,-0.075f, 0.075f,-0.075f,
              0.075f,0.055f, -0.075f,0.055f);

        // Side wall shading.
        color(wallR * 0.78f, wallG * 0.78f, wallB * 0.78f);
        quad(0.025f,-0.075f, 0.075f,-0.075f,
             0.075f,0.055f, 0.025f,0.055f);

        // Roof.
        color(0.40f, 0.16f, 0.14f);
        triangle(-0.095f,0.045f, 0.0f,0.145f, 0.095f,0.045f);

        color(0.65f, 0.25f, 0.18f);
        triangle(-0.095f,0.045f, 0.0f,0.145f, 0.0f,0.045f);

        // Door.
        color(0.25f, 0.15f, 0.10f);
        quad(-0.018f,-0.075f, 0.018f,-0.075f,
             0.018f,0.005f, -0.018f,0.005f);

        // Windows.
        color(0.62f, 0.85f, 0.91f);
        quad(-0.060f,-0.005f, -0.035f,-0.005f,
             -0.035f,0.025f, -0.060f,0.025f);
        quad(0.038f,-0.005f, 0.063f,-0.005f,
             0.063f,0.025f, 0.038f,0.025f);

        // Window frames.
        color(0.28f, 0.20f, 0.14f);
        line(-0.047f,-0.005f, -0.047f,0.025f, 1.0f);
        line(0.050f,-0.005f, 0.050f,0.025f, 1.0f);

        glPopMatrix();
    }

    void village()
    {
        // Village sits on the far central valley terrace.
        // Rear houses first, then larger front houses.
        house(-0.31f, -0.285f, 0.28f, 0.83f, 0.68f, 0.48f);
        house(-0.19f, -0.275f, 0.34f, 0.75f, 0.58f, 0.43f);
        house(-0.04f, -0.300f, 0.30f, 0.88f, 0.75f, 0.56f);
        house(0.12f, -0.285f, 0.34f, 0.77f, 0.63f, 0.48f);
        house(0.27f, -0.305f, 0.29f, 0.85f, 0.70f, 0.52f);
        house(0.40f, -0.315f, 0.25f, 0.78f, 0.61f, 0.45f);

        house(-0.23f, -0.365f, 0.25f, 0.88f, 0.74f, 0.56f);
        house(-0.06f, -0.375f, 0.27f, 0.80f, 0.64f, 0.46f);
        house(0.13f, -0.375f, 0.24f, 0.91f, 0.77f, 0.59f);
        house(0.30f, -0.380f, 0.22f, 0.77f, 0.61f, 0.47f);

    }

    void pineTree(float x, float y, float s)
    {
        glPushMatrix();
        glTranslatef(x, y, 0.0f);
        glScalef(s, s, 1.0f);

        color(0.22f, 0.14f, 0.10f);
        quad(-0.012f,-0.08f, 0.012f,-0.08f,
             0.012f,0.12f, -0.012f,0.12f);

        color(0.035f, 0.22f, 0.16f);
        triangle(-0.10f,0.02f, 0.0f,0.28f, 0.10f,0.02f);
        triangle(-0.085f,0.10f, 0.0f,0.36f, 0.085f,0.10f);
        triangle(-0.07f,0.18f, 0.0f,0.43f, 0.07f,0.18f);

        color(0.10f, 0.34f, 0.22f);
        triangle(-0.04f,0.20f, 0.0f,0.43f, 0.025f,0.20f);

        glPopMatrix();
    }

    void villageTrees()
    {
        pineTree(-0.42f, -0.34f, 0.35f);
        pineTree(0.48f, -0.35f, 0.32f);
        pineTree(-0.34f, -0.43f, 0.25f);
        pineTree(0.39f, -0.44f, 0.24f);
    }

    void cliffFaces()
    {
        // Dark left cliff mass.
        color(0.055f, 0.13f, 0.19f);
        const float leftCliff[] = {
            -1.25f,-1.2f, -1.25f,0.08f, -1.03f,0.16f,
            -0.91f,0.04f, -0.86f,-0.18f, -0.78f,-0.34f,
            -0.72f,-0.62f, -0.55f,-1.2f
        };
        polygon(leftCliff, 8);

        // Left lit rock plane.
        color(0.18f, 0.27f, 0.29f);
        const float leftRock[] = {
            -1.02f,0.12f, -0.91f,0.04f, -0.86f,-0.18f,
            -0.78f,-0.34f, -0.83f,-0.62f, -0.94f,-0.40f
        };
        polygon(leftRock, 6);

        color(0.31f, 0.34f, 0.31f);
        const float leftHighlight[] = {
            -0.98f,0.08f, -0.91f,0.04f, -0.86f,-0.18f,
            -0.91f,-0.28f, -0.96f,-0.12f
        };
        polygon(leftHighlight, 5);

        // Right cliff mass.
        color(0.045f, 0.12f, 0.17f);
        const float rightCliff[] = {
            1.25f,-1.2f, 1.25f,0.14f, 1.04f,0.22f,
            0.92f,0.06f, 0.88f,-0.17f, 0.80f,-0.36f,
            0.73f,-0.65f, 0.55f,-1.2f
        };
        polygon(rightCliff, 8);

        // Right exposed rock face.
        color(0.16f, 0.25f, 0.27f);
        const float rightRock[] = {
            1.04f,0.18f, 0.92f,0.06f, 0.88f,-0.17f,
            0.80f,-0.36f, 0.85f,-0.63f, 0.98f,-0.39f
        };
        polygon(rightRock, 6);

        color(0.28f, 0.32f, 0.29f);
        const float rightHighlight[] = {
            1.00f,0.13f, 0.92f,0.06f, 0.88f,-0.17f,
            0.93f,-0.27f, 0.98f,-0.10f
        };
        polygon(rightHighlight, 5);

        // Angular rock strata.
        color(0.36f, 0.39f, 0.34f);
        line(-0.94f,-0.20f, -0.82f,-0.34f, 2.0f);
        line(-0.89f,-0.39f, -0.77f,-0.51f, 1.5f);
        line(-0.91f,-0.55f, -0.75f,-0.68f, 1.5f);
        line(0.94f,-0.18f, 0.82f,-0.34f, 2.0f);
        line(0.89f,-0.39f, 0.77f,-0.52f, 1.5f);
        line(0.91f,-0.56f, 0.75f,-0.69f, 1.5f);

        // Green grassy cliff rims.
        color(0.30f, 0.55f, 0.25f);
        curve(-1.20f,0.09f, -1.07f,0.18f, -0.91f,0.04f, 5.0f);
        curve(1.20f,0.15f, 1.08f,0.24f, 0.92f,0.06f, 5.0f);
        color(0.57f, 0.67f, 0.31f);
        curve(-1.17f,0.10f, -1.06f,0.17f, -0.94f,0.07f, 1.5f);
        curve(1.17f,0.16f, 1.07f,0.22f, 0.95f,0.08f, 1.5f);
    }

    void foregroundVegetation()
    {
        // Clusters of pine trees along the slopes.
        pineTree(-0.96f, -0.28f, 0.72f);
        pineTree(-0.84f, -0.48f, 0.58f);
        pineTree(-0.73f, -0.64f, 0.48f);
        pineTree(0.97f, -0.27f, 0.72f);
        pineTree(0.84f, -0.47f, 0.58f);
        pineTree(0.72f, -0.64f, 0.48f);

        // Small bushes.
        color(0.10f, 0.30f, 0.17f);
        disk(-0.61f,-0.78f,0.075f,0.035f);
        disk(-0.53f,-0.82f,0.060f,0.030f);
        disk(0.60f,-0.79f,0.075f,0.035f);
        disk(0.52f,-0.83f,0.060f,0.030f);

        color(0.48f, 0.63f, 0.28f);
        line(-0.67f,-0.78f,-0.65f,-0.73f,1.0f);
        line(-0.57f,-0.82f,-0.55f,-0.77f,1.0f);
        line(0.65f,-0.79f,0.67f,-0.74f,1.0f);
        line(0.56f,-0.82f,0.58f,-0.77f,1.0f);
    }
}

void drawMountains(float zoom, float parallax)
{
    // Clamp values to keep the camera inside a useful range.
    zoom = std::max(1.0f, std::min(zoom, 2.25f));
    parallax = std::max(0.0f, std::min(parallax, 1.0f));

    glPushMatrix();

    // Camera moves toward the village around the valley center.
    // This makes the houses grow toward the center rather than zooming
    // toward an arbitrary point in the scene.
    const float focusX = 0.02f;
    const float focusY = -0.34f;
    glTranslatef(-focusX * (zoom - 1.0f),
                 -focusY * (zoom - 1.0f), 0.0f);
    glScalef(zoom, zoom, 1.0f);

    skyAndClouds();

    // Far mountains have the slowest parallax.
    glPushMatrix();
    glTranslatef(-0.015f * parallax, 0.0f, 0.0f);
    distantMountains();
    glPopMatrix();

    // Middle ridges move a little more.
    glPushMatrix();
    glTranslatef(-0.045f * parallax, 0.0f, 0.0f);
    middleRanges();
    glPopMatrix();

    valleyFloor();
    riverAndBanks();
    windingRoad();
    village();
    villageTrees();

    // Near cliffs and plants move the most.
    glPushMatrix();
    glTranslatef(-0.11f * parallax, 0.0f, 0.0f);
    cliffFaces();
    foregroundVegetation();
    glPopMatrix();

    glPopMatrix();
}
