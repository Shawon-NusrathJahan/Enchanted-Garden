#include "mountain.h"
#include "gfx.h"
#include "village.h"

#include <algorithm>
#include <utility>
#include <vector>

using namespace gfx;

namespace
{

// ---------------------------------------------------------------------------
// Camera. Every layer is scaled about the pivot P (the village). A layer's
// "depth factor" f decides how strongly it reacts:
//     scale_i = 1 + (zoom - 1) * f_i          (forward journey)
//     pan_i   = -camX * f_i * PARALLAX        (slow sideways drift)
// f = 0 is infinitely far away, f = 1 is the village, f > 1 is in front of it.
// ---------------------------------------------------------------------------
const float PX = 0.0f, PY = -0.09f;
const float PARALLAX = 2.2f;

const float F_SKY    = 0.04f;
const float F_FAR0   = 0.06f;     // snowy giants
const float F_FAR_A  = 0.12f;     // blue alpine ridge
const float F_FAR_B  = 0.22f;     // grey-violet rocky crags
const float F_MID    = 0.34f;     // green rounded mountains
const float F_HILLS  = 0.55f;     // forested hills
const float F_VEIL2  = 0.66f;     // big near mountains that hide the village at first
const float F_VEIL1  = 0.80f;     // front forest ridge
const float F_VALLEY = 1.00f;
const float F_CLIFF  = 1.60f;     // hillsides framing the shot

const float VEIL2_DROP = 1.8f;    // how far the near ridges slide down (screen units)
const float VEIL1_DROP = 2.0f;

const float APEX_X     = 0.75f;   // inner top of each framing hillside
const float HILL_SLOPE = 0.45f;   // inner diagonal: d = (top0 - y) * slope

struct Pine { float x, y, h, phase; };

Ridge far0, farA, farB, midR, hillsR, veil2R, veil1R, valleyTop;
std::vector<Pine> midPines, hillPines, veil2Pines, veil1Pines, valleyPinesBack, valleyPinesFront;

// One framing hillside, stored in "outward distance" d from its inner top so
// the left and right sides share the same code (sign = -1 / +1).
struct CliffHalf
{
    float sign, L, top0;
    std::vector<float> d, top;
    std::vector<float> faceY, faceIn, faceOut;
    std::vector<std::vector<std::pair<float, float>>> cracks;
    std::vector<Pine> trees, rim;
};
CliffHalf cliffL, cliffR;

float edgeAt(float y, float top0) { return (top0 - y) * HILL_SLOPE; }
float lowAt(float d, float top0)  { return std::max(top0 - d / HILL_SLOPE, -2.4f); }

float topAt(const CliffHalf& c, float d)
{
    float f = clampf(d / c.L * (c.d.size() - 1), 0.0f, (float)c.d.size() - 1.001f);
    const int i = (int)f;
    return lerpf(c.top[i], c.top[i + 1], f - i);
}

CliffHalf makeCliff(float sign, float seed)
{
    CliffHalf c;
    c.sign = sign;
    c.L    = WORLD_W + 0.9f;

    const int N = 100;
    for (int i = 0; i <= N; i++)
    {
        const float u = (float)i / N;
        const float y = 0.10f + 0.42f * std::pow(u, 0.85f) + 0.030f * std::sin(11.0f * u + seed) +
                        0.020f * std::sin(27.0f * u + seed * 2.0f) + 0.010f * std::sin(63.0f * u + seed * 3.0f);
        c.d.push_back(u * c.L);
        c.top.push_back(y);
    }
    c.top0 = c.top[0];

    const int M = 26;
    for (int j = 0; j <= M; j++)
    {
        const float y   = c.top0 - 0.002f - 0.75f * j / M;
        const float jag = 0.02f + 0.07f * hash1(j * 3.7f + seed);
        c.faceY.push_back(y);
        c.faceIn.push_back(edgeAt(y, c.top0) - 0.02f - jag);
        c.faceOut.push_back(edgeAt(y, c.top0) + 0.34f + 0.14f * hash1(j * 1.9f + seed * 5.0f));
    }

    for (int k = 0; k < 6; k++)
    {
        std::vector<std::pair<float, float>> crack;
        float y = c.top0 - 0.04f - 0.12f * hash1(k * 2.1f + seed);
        float off = 0.05f + 0.20f * hash1(k * 5.3f + seed);
        for (int p = 0; p < 4; p++)
        {
            crack.push_back({edgeAt(y, c.top0) + off, y});
            off += (hash1(k * 9.1f + p + seed) - 0.5f) * 0.05f;
            y -= 0.07f + 0.08f * hash1(k * 4.4f + p * 1.3f);
        }
        c.cracks.push_back(crack);
    }

    for (int i = 0; i < 150; i++)
    {
        const float d  = 1.95f * std::pow(hash1(i * 7.13f + seed), 1.2f);
        const float yb = lowAt(d, c.top0), yt = topAt(c, d) - 0.04f;
        if (yt - yb < 0.08f) continue;
        const float y = lerpf(yb + 0.02f, yt, hash1(i * 3.91f + seed * 2.0f));
        const float depth = clampf((yt - y) / 1.0f, 0.0f, 1.0f);
        c.trees.push_back({d, y, 0.08f + 0.08f * hash1(i * 5.5f + seed) + 0.10f * depth, hash1(i * 1.3f)});
    }
    std::stable_sort(c.trees.begin(), c.trees.end(), [](const Pine& a, const Pine& b) { return a.y > b.y; });

    for (float d = 0.45f; d < c.L - 0.05f; d += 0.22f + 0.15f * hash1(d * 17.0f + seed))
        c.rim.push_back({d, topAt(c, d) - 0.02f, 0.17f + 0.12f * hash1(d * 5.0f + seed), hash1(d * 3.0f)});
    return c;
}

void drawCliff(const CliffHalf& c, float t)
{
    const float s = c.sign;
    const int   n = (int)c.d.size();

    for (int band = 0; band < 2; band++)
    {
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i < n; i++)
        {
            const float y    = c.top[i];
            const float ybot = lowAt(c.d[i], c.top0);
            const float y1   = std::max(y - 0.35f, ybot);
            const float ya = band == 0 ? y : y1;
            const float yb = band == 0 ? y1 : ybot;
            const Color ca = band == 0 ? rgb(0.44f, 0.72f, 0.28f) : rgb(0.19f, 0.44f, 0.25f);
            const Color cb = band == 0 ? rgb(0.19f, 0.44f, 0.25f) : rgb(0.05f, 0.20f, 0.17f);
            vtx(ca, s * c.d[i], ya);
            vtx(cb, s * c.d[i], yb);
        }
        glEnd();
    }
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i < n; i++)
    {
        vtx(rgb(0.72f, 0.90f, 0.38f), s * c.d[i], c.top[i]);
        vtx(rgb(0.46f, 0.76f, 0.30f), s * c.d[i], c.top[i] - 0.05f - 0.02f * std::sin(c.d[i] * 38.0f));
    }
    glEnd();

    const int m = (int)c.faceY.size();
    glBegin(GL_TRIANGLE_STRIP);
    for (int j = 0; j < m; j++)
    {
        const float band = 0.90f + 0.10f * hash1(j * 7.7f + s);
        const float dark = 1.0f - 0.30f * (float)j / m;
        vtx(shade(rgb(0.70f, 0.80f, 0.88f), band * dark), s * c.faceIn[j], c.faceY[j]);
        vtx(shade(rgb(0.30f, 0.42f, 0.56f), band * dark), s * c.faceOut[j], c.faceY[j]);
    }
    glEnd();
    glLineWidth(2.0f);
    setColor(rgb(0.14f, 0.20f, 0.30f, 0.75f));
    for (const auto& crack : c.cracks)
    {
        glBegin(GL_LINE_STRIP);
        for (const auto& p : crack) glVertex2f(s * p.first, p.second);
        glEnd();
    }
    glLineWidth(3.0f);
    setColor(rgb(0.08f, 0.12f, 0.20f, 0.9f));
    glBegin(GL_LINE_STRIP);
    for (int j = 0; j < m; j++) glVertex2f(s * c.faceIn[j], c.faceY[j]);
    glEnd();
    glLineWidth(1.0f);

    for (const Pine& p : c.trees)
        drawPine(s * p.x, p.y, p.h, 0.010f * std::sin(t * 1.1f + p.phase * 6.0f), 0.55f + 0.35f * p.phase);
    for (const Pine& p : c.rim)
        drawPine(s * p.x, p.y, p.h, 0.012f * std::sin(t * 1.1f + p.phase * 6.0f), 0.95f);
}

// ---------------------------------------------------------------------------
// Layer helpers
// ---------------------------------------------------------------------------
void beginLayer(float f, const AnimState& s, float slideDown = 0.0f)
{
    const float scale = 1.0f + (s.zoom - 1.0f) * f;
    const float panX  = -s.camX * f * PARALLAX;
    const float panY  = -PY * f * s.travel - slideDown;    // journey brings the village to centre
    pushCamera(scale, PX, PY, panX, panY);
}

float wrap(float v, float lo, float hi)
{
    const float w = hi - lo;
    return lo + std::fmod(std::fmod(v - lo, w) + w, w);
}

void drawPines(const std::vector<Pine>& list, float t, float dark)
{
    for (const Pine& p : list)
        if (visible(p.x, p.y + p.h * 0.5f, p.h * 0.5f))
            drawPine(p.x, p.y, p.h, 0.008f * std::sin(t + p.phase * 6.0f), dark);
}

// ---------------------------------------------------------------------------
// Scene parts
// ---------------------------------------------------------------------------
void drawSky(float t)
{
    static const Stop stops[] = {
        {-1.6f, rgb(0.99f, 0.90f, 0.82f)}, {-0.1f, rgb(0.99f, 0.90f, 0.82f)}, {0.12f, rgb(0.80f, 0.89f, 0.96f)},
        {0.45f, rgb(0.50f, 0.74f, 0.94f)}, {0.80f, rgb(0.28f, 0.52f, 0.88f)}, {1.60f, rgb(0.13f, 0.30f, 0.68f)},
        {2.40f, rgb(0.13f, 0.30f, 0.68f)}};
    vGradient(-3.2f, 3.2f, stops, 7);

    glow(0.10f, 0.28f, 1.35f, rgb(1.0f, 0.88f, 0.70f, 0.85f), 48);
    glow(0.10f, 0.28f, 0.50f, rgb(1.0f, 0.96f, 0.85f, 0.9f), 40);

    struct C { float x, y, s, speed, pink; };
    static const C clouds[] = {{-1.30f, 0.84f, 0.85f, 0.010f, 0.85f}, {1.35f, 0.84f, 0.95f, 0.007f, 0.80f},
                               {0.15f, 0.86f, 0.45f, 0.012f, 0.55f},  {-0.55f, 0.90f, 0.35f, 0.015f, 0.40f},
                               {0.80f, 0.90f, 0.38f, 0.009f, 0.45f}};
    for (const C& c : clouds) drawCloud(wrap(c.x + c.speed * t, -3.2f, 3.2f), c.y, c.s, c.pink);

    for (int i = 0; i < 5; i++)
    {
        const float bx = wrap(-1.4f + i * 0.45f + 0.06f * t, -3.0f, 3.0f);
        const float by = 0.70f + 0.05f * std::sin(i * 2.3f + t * 0.7f) + 0.04f * i;
        drawBird(bx, by, 0.024f, std::sin(t * 7.0f + i * 1.7f), rgb(0.16f, 0.20f, 0.32f));
    }
}

void drawValley(float t, const AnimState& s)
{
    drawRidge(valleyTop, -1.9f, rgb(0.62f, 0.78f, 0.42f), rgb(0.20f, 0.45f, 0.22f));

    struct Blob { float x, y, rx, ry; Color c; };
    static const Blob blobs[] = {
        {-1.20f, -0.30f, 0.95f, 0.26f, rgb(0.74f, 0.84f, 0.44f, 0.75f)},
        {1.25f, -0.38f, 0.95f, 0.24f, rgb(0.52f, 0.72f, 0.34f, 0.75f)},
        {-0.40f, -0.75f, 1.00f, 0.30f, rgb(0.40f, 0.64f, 0.30f, 0.65f)},
        {0.55f, -0.95f, 1.20f, 0.30f, rgb(0.52f, 0.72f, 0.34f, 0.65f)},
        {-1.50f, -0.85f, 0.90f, 0.32f, rgb(0.30f, 0.55f, 0.26f, 0.65f)}};
    for (const Blob& b : blobs)
    {
        glBegin(GL_TRIANGLE_FAN);
        setColor(b.c);
        glVertex2f(b.x, b.y);
        glColor4f(b.c.r, b.c.g, b.c.b, 0.0f);
        for (int i = 0; i <= 32; i++)
            glVertex2f(b.x + std::cos(TAU * i / 32) * b.rx, b.y + std::sin(TAU * i / 32) * b.ry);
        glEnd();
    }

    struct Field { float yt, xl0, xr0, yb, xl1, xr1; Color base, row; };
    static const Field fields[] = {
        {-0.19f, 0.40f, 0.80f, -0.64f, 0.46f, 1.28f, rgb(0.88f, 0.80f, 0.48f), rgb(0.78f, 0.68f, 0.36f)},
        {-0.21f, -1.00f, -0.58f, -0.66f, -1.42f, -0.70f, rgb(0.64f, 0.80f, 0.40f), rgb(0.52f, 0.70f, 0.32f)}};
    for (const Field& f : fields)
    {
        setColor(f.base);
        quad(f.xl0, f.yt, f.xr0, f.yt, f.xr1, f.yb, f.xl1, f.yb);
        setColor(f.row);
        for (int i = 1; i < 10; i++)
        {
            const float k = i / 10.0f;
            thickLine(lerpf(f.xl0, f.xr0, k), f.yt, lerpf(f.xl1, f.xr1, k), f.yb, 0.006f);
        }
    }

    setColor(rgb(0.66f, 0.80f, 0.44f, 0.75f));
    ellipse(0.0f, -0.085f, 0.70f, 0.15f, 40);
    setColor(rgb(0.80f, 0.70f, 0.48f, 0.55f));
    ellipse(0.0f, -0.17f, 0.42f, 0.04f, 40);

    const int RN = 40;
    auto riverX = [](float v) { return -0.72f - 0.50f * v + 0.15f * std::sin(5.0f * v); };
    auto riverY = [](float v) { return 0.02f - 1.75f * v; };
    auto riverW = [](float v) { return 0.010f + 0.085f * v * v + 0.02f * v; };
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= RN; i++)
    {
        const float v = (float)i / RN, w = riverW(v) * 1.35f;
        vtx(rgb(0.60f, 0.74f, 0.46f), riverX(v) - w, riverY(v));
        vtx(rgb(0.60f, 0.74f, 0.46f), riverX(v) + w, riverY(v));
    }
    glEnd();
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= RN; i++)
    {
        const float v = (float)i / RN, w = riverW(v);
        vtx(rgb(0.50f, 0.80f, 0.92f), riverX(v) - w, riverY(v));
        vtx(rgb(0.30f, 0.62f, 0.82f), riverX(v) + w, riverY(v));
    }
    glEnd();
    for (int i = 0; i < 8; i++)
    {
        const float v = fract(t * 0.05f + i / 8.0f);
        setColor(rgb(1.0f, 1.0f, 1.0f, 0.55f * std::sin(v * PI)));
        ellipse(riverX(v), riverY(v), riverW(v) * 0.55f, 0.004f + 0.006f * v, 10);
    }

    const int PN = 40;
    auto pathX = [](float v) { return -0.06f + 0.70f * v + 0.18f * v * std::sin(4.0f * v + 0.5f); };
    auto pathY = [](float v) { return -0.10f - 1.55f * v; };
    auto pathW = [](float v) { return 0.026f + 0.17f * v; };
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= PN; i++)
    {
        const float v = (float)i / PN, w = pathW(v) * 1.25f;
        vtx(rgb(0.70f, 0.58f, 0.38f), pathX(v) - w, pathY(v));
        vtx(rgb(0.70f, 0.58f, 0.38f), pathX(v) + w, pathY(v));
    }
    glEnd();
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= PN; i++)
    {
        const float v = (float)i / PN, w = pathW(v);
        vtx(rgb(0.93f, 0.84f, 0.60f), pathX(v) - w, pathY(v));
        vtx(rgb(0.86f, 0.74f, 0.50f), pathX(v) + w, pathY(v));
    }
    glEnd();

    for (const Pine& p : valleyPinesBack)
        if (visible(p.x, p.y + p.h * 0.5f, p.h * 0.5f))
            drawPine(p.x, p.y, p.h, 0.012f * std::sin(t * 1.2f + p.phase * 6.0f));
    drawVillage(s.time);
    for (const Pine& p : valleyPinesFront)
        if (visible(p.x, p.y + p.h * 0.5f, p.h * 0.5f))
            drawPine(p.x, p.y, p.h, 0.012f * std::sin(t * 1.2f + p.phase * 6.0f));
}

}   // namespace

// ---------------------------------------------------------------------------
void initMountain()
{
    // 1) snowy alpine giants, very pale and far away
    far0 = makePeakRidge(-3.0f, 3.0f, 0.02f, 0.10f,
                         {{-2.3f, 0.52f, 0.65f}, {-1.45f, 0.68f, 0.60f}, {-0.55f, 0.56f, 0.55f},
                          {0.35f, 0.74f, 0.65f}, {1.30f, 0.60f, 0.60f}, {2.25f, 0.54f, 0.65f}},
                         2.1f, 0.05f, false);

    // 2) sharp blue ridge   3) grey-violet rocky crags
    farA = makeRidge(-3.0f, 3.0f, 0.02f, 0.10f, 0.52f, 1.3f, 2.4f);
    farB = makePeakRidge(-3.0f, 3.0f, 0.02f, 0.04f,
                         {{-2.5f, 0.40f, 0.42f}, {-1.7f, 0.50f, 0.40f}, {-0.9f, 0.34f, 0.45f}, {-0.1f, 0.46f, 0.38f},
                          {0.7f, 0.52f, 0.42f}, {1.6f, 0.38f, 0.45f}, {2.4f, 0.48f, 0.42f}},
                         5.7f, 0.07f, false);

    // 4) green rounded mountains with pine texture
    midR = makePeakRidge(-3.0f, 3.0f, 0.02f, 0.00f,
                         {{-2.3f, 0.40f, 0.75f}, {-1.1f, 0.33f, 0.65f}, {0.0f, 0.28f, 0.60f},
                          {1.2f, 0.42f, 0.75f}, {2.4f, 0.35f, 0.70f}},
                         3.3f, 0.04f, true);

    hillsR    = makeRidge(-3.0f, 3.0f, 0.02f, -0.02f, 0.24f, 2.2f, 5.5f);
    valleyTop = makeRolling(-3.0f, 3.0f, 0.02f, 0.00f, 0.07f, 1.1f, 2.8f);

    // 5) big near mountains (they hide the village at the start)
    veil2R = makePeakRidge(-3.0f, 3.0f, 0.02f, 0.26f,
                           {{-1.9f, 0.32f, 0.85f}, {-0.6f, 0.22f, 0.75f}, {0.5f, 0.34f, 0.80f}, {1.7f, 0.26f, 0.85f}},
                           8.9f, 0.05f, false);
    veil1R = makeRolling(-3.0f, 3.0f, 0.02f, 0.18f, 0.22f, 4.4f, 3.1f);

    auto scatter = [](std::vector<Pine>& out, const Ridge& r, float step, float hMin, float hVar, float dy, float seed)
    {
        for (float x = -2.9f; x < 2.9f; x += step * (0.6f + 0.8f * hash1(x * 31.7f + seed)))
            out.push_back({x, ridgeAt(r, x) - dy - 0.02f * hash1(x * 5.1f), hMin + hVar * hash1(x * 7.3f + seed), hash1(x * 3.1f)});
    };
    midPines.clear(); hillPines.clear(); veil2Pines.clear(); veil1Pines.clear();
    scatter(midPines, midR, 0.05f, 0.035f, 0.035f, 0.010f, 1.0f);
    scatter(hillPines, hillsR, 0.04f, 0.055f, 0.055f, 0.015f, 2.0f);
    scatter(veil2Pines, veil2R, 0.06f, 0.07f, 0.06f, 0.010f, 3.0f);
    // forest slope: rows from the crest downward, bigger trees lower on the screen
    for (int row = 0; row < 7; row++)
    {
        const float dy = 0.02f + 0.17f * row;
        scatter(veil1Pines, veil1R, 0.10f - 0.006f * row, 0.12f + 0.025f * row, 0.08f + 0.02f * row, dy, 4.0f + row);
    }
    std::stable_sort(veil1Pines.begin(), veil1Pines.end(), [](const Pine& a, const Pine& b) { return a.y > b.y; });

    valleyPinesBack = {{-1.00f, -0.10f, 0.12f, 0.1f}, {-0.84f, -0.08f, 0.10f, 0.4f}, {0.90f, -0.10f, 0.12f, 0.7f},
                       {0.76f, -0.07f, 0.10f, 0.2f},  {-0.70f, -0.06f, 0.11f, 0.9f}, {0.72f, -0.05f, 0.11f, 0.5f},
                       {-1.15f, -0.06f, 0.10f, 0.3f}, {1.10f, -0.06f, 0.11f, 0.8f}};
    valleyPinesFront = {{-0.95f, -0.22f, 0.20f, 0.3f}, {-0.82f, -0.30f, 0.16f, 0.6f}, {-1.02f, -0.38f, 0.22f, 0.8f},
                        {-0.72f, -0.20f, 0.12f, 0.1f}, {-0.90f, -0.50f, 0.24f, 0.4f}, {-0.62f, -0.16f, 0.10f, 0.7f},
                        {-1.15f, -0.25f, 0.17f, 0.2f}, {-1.20f, -0.55f, 0.26f, 0.5f},
                        {0.92f, -0.24f, 0.20f, 0.5f},  {0.80f, -0.31f, 0.15f, 0.2f},  {1.02f, -0.40f, 0.22f, 0.9f},
                        {0.70f, -0.20f, 0.11f, 0.6f},  {0.88f, -0.52f, 0.25f, 0.3f},  {0.64f, -0.18f, 0.10f, 0.1f},
                        {1.15f, -0.27f, 0.17f, 0.4f},  {1.22f, -0.58f, 0.27f, 0.7f}};

    cliffL = makeCliff(-1.0f, 1.7f);
    cliffR = makeCliff(+1.0f, 4.9f);

    initVillage();
}

void drawMountainScene(const AnimState& s)
{
    const float t = (float)s.time;

    // sky, warm glow, clouds, birds
    beginLayer(F_SKY, s);
    drawSky(t);
    popCamera();

    // snowy giants (pale, lilac)
    beginLayer(F_FAR0, s);
    drawRidge(far0, -0.6f, rgb(0.60f, 0.66f, 0.90f), rgb(0.86f, 0.91f, 0.99f));
    drawRidgeLights(far0, rgb(0.95f, 0.97f, 1.00f, 0.35f));
    drawSnow(far0, 0.46f, rgb(1.00f, 1.00f, 1.00f), rgb(0.86f, 0.90f, 1.00f));
    drawRidgeShadows(far0, rgb(0.46f, 0.52f, 0.82f, 0.50f));
    popCamera();

    // blue alpine ridge
    beginLayer(F_FAR_A, s);
    drawRidge(farA, -0.6f, rgb(0.46f, 0.66f, 0.88f), rgb(0.80f, 0.89f, 0.96f));
    drawRidgeLights(farA, rgb(0.86f, 0.93f, 1.00f, 0.30f));
    drawRidgeShadows(farA, rgb(0.34f, 0.54f, 0.80f, 0.55f));
    popCamera();

    // grey-violet rocky crags with gullies
    beginLayer(F_FAR_B, s);
    drawRidge(farB, -0.6f, rgb(0.42f, 0.50f, 0.72f), rgb(0.72f, 0.80f, 0.92f));
    drawRidgeLights(farB, rgb(0.84f, 0.88f, 0.98f, 0.30f));
    drawRidgeShadows(farB, rgb(0.20f, 0.28f, 0.58f, 0.55f));
    drawRockStreaks(farB, rgb(0.18f, 0.24f, 0.50f, 0.30f), 0.22f);
    popCamera();

    // green rounded mountains covered in pines
    beginLayer(F_MID, s);
    drawRidge(midR, -0.7f, rgb(0.18f, 0.50f, 0.38f), rgb(0.52f, 0.76f, 0.66f));
    drawRidgeLights(midR, rgb(0.62f, 0.88f, 0.52f, 0.40f));
    drawRidgeShadows(midR, rgb(0.05f, 0.28f, 0.30f, 0.55f));
    drawPines(midPines, t, 0.75f);
    popCamera();

    // forested hills + valley mist
    beginLayer(F_HILLS, s);
    drawRidge(hillsR, -0.8f, rgb(0.16f, 0.48f, 0.36f), rgb(0.38f, 0.68f, 0.46f));
    drawRidgeLights(hillsR, rgb(0.62f, 0.86f, 0.50f, 0.30f));
    drawRidgeShadows(hillsR, rgb(0.05f, 0.28f, 0.28f, 0.55f));
    drawPines(hillPines, t, 0.80f);
    rectV(-3.0f, -0.05f, 3.0f, 0.28f, rgb(1.0f, 0.95f, 0.88f, 0.42f), rgb(1.0f, 0.95f, 0.88f, 0.0f));
    popCamera();

    // valley floor, river, road, village
    beginLayer(F_VALLEY, s);
    drawValley(t, s);
    popCamera();

    // near mountains that hide the village at first and slide down to reveal it
    if (s.open < 0.999f)
    {
        beginLayer(F_VEIL2, s, VEIL2_DROP * s.open);
        drawRidge(veil2R, -1.0f, rgb(0.10f, 0.38f, 0.38f), rgb(0.26f, 0.56f, 0.48f));
        drawRidgeLights(veil2R, rgb(0.58f, 0.84f, 0.55f, 0.35f));
        drawRidgeShadows(veil2R, rgb(0.02f, 0.20f, 0.26f, 0.60f));
        drawRockStreaks(veil2R, rgb(0.02f, 0.18f, 0.24f, 0.30f), 0.40f);
        drawPines(veil2Pines, t, 0.70f);
        popCamera();

        beginLayer(F_VEIL1, s, VEIL1_DROP * s.open);
        drawRidge(veil1R, -1.6f, rgb(0.30f, 0.60f, 0.30f), rgb(0.05f, 0.22f, 0.16f));
        drawPines(veil1Pines, t, 0.75f);
        popCamera();
    }

    // framing hillsides (forest + rock outcrop), as in the reference picture
    beginLayer(F_CLIFF, s);
    if (toScreenX(-APEX_X + 0.12f) > -WORLD_W - 0.1f)
    {
        glPushMatrix();
        glTranslatef(-APEX_X, 0.0f, 0.0f);
        drawCliff(cliffL, t);
        glPopMatrix();
        ++statDrawn;
    }
    else ++statCulled;

    if (toScreenX(APEX_X - 0.12f) < WORLD_W + 0.1f)
    {
        glPushMatrix();
        glTranslatef(APEX_X, 0.0f, 0.0f);
        drawCliff(cliffR, t);
        glPopMatrix();
        ++statDrawn;
    }
    else ++statCulled;
    popCamera();
}
