#include <graphics.h>
#include <math.h>
#include <stdlib.h>

#define PI 3.14159265
#define COL(r,g,b) COLOR(r,g,b)

#define G_DARK   COL(30,115,35)
#define G_MID    COL(60,160,40)
#define G_LIGHT  COL(120,200,50)
#define BROWN    COL(140,85,40)
#define BROWN_D  COL(90,50,20)

void colorOnly(int c) { setcolor(c); setfillstyle(SOLID_FILL, c); }

void colorPair(int line, int fill) { setcolor(line); setfillstyle(SOLID_FILL, fill); }

void ell(int x, int y, int rx, int ry, int c)
{
    colorOnly(c);
    fillellipse(x, y, rx, ry);
}

void ellO(int x, int y, int rx, int ry, int fill, int line)
{
    colorPair(line, fill);
    fillellipse(x, y, rx, ry);
}

void box(int x1, int y1, int x2, int y2, int c)
{
    colorOnly(c);
    bar(x1, y1, x2, y2);
}

void boxO(int x1, int y1, int x2, int y2, int fill, int line)
{
    colorPair(line, fill);
    bar(x1, y1, x2, y2);
    rectangle(x1, y1, x2, y2);
}

void poly(int n, int *p, int c)
{
    colorOnly(c);
    fillpoly(n, p);
}

void polyO(int n, int *p, int fill, int line)
{
    colorPair(line, fill);
    fillpoly(n, p);
}

void tri(int x1, int y1, int x2, int y2, int x3, int y3, int c)
{
    int p[6] = {x1, y1, x2, y2, x3, y3};
    poly(3, p, c);
}

void thickLine(int x1, int y1, int x2, int y2, int w, int c)
{
    setcolor(c);
    setlinestyle(SOLID_LINE, 0, w);
    line(x1, y1, x2, y2);
    setlinestyle(SOLID_LINE, 0, 1);
}

void petal(int x, int y, double ang, int len, int wid, int fill, int line)
{
    double a = ang * PI / 180.0;
    int tx = x + (int)(len * cos(a));
    int ty = y + (int)(len * sin(a));
    int mx = x + (int)(len * 0.55 * cos(a));
    int my = y + (int)(len * 0.55 * sin(a));
    int nx = (int)(-sin(a) * wid);
    int ny = (int)(cos(a) * wid);
    int p[8] = {x, y, mx + nx, my + ny, tx, ty, mx - nx, my - ny};
    polyO(4, p, fill, line);
}

void drawLeafCluster(int cx, int cy, int r, int c)
{
    ell(cx, cy, r, r, c);
    for (int i = 0; i < 6; i++)
    {
        double a = i * 60 * PI / 180.0 + (cx % 7) * 0.2;
        ell(cx + (int)(r * 0.8 * cos(a)), cy + (int)(r * 0.8 * sin(a)),
            r / 2 + 1, r / 2 + 1, c);
    }
}

void scatter(int n, int cx, int cy, int rx, int ry, int r, int c)
{
    for (int i = 0; i < n; i++)
    {
        int x, y;
        do
        {
            x = cx - rx + rand() % (2 * rx + 1);
            y = cy - ry + rand() % (2 * ry + 1);
        } while (((double)(x - cx) * (x - cx)) / ((double)rx * rx) +
                 ((double)(y - cy) * (y - cy)) / ((double)ry * ry) > 1.0);
        drawLeafCluster(x, y, r, c);
    }
}


void drawSky()
{
    for (int y = 0; y < 340; y += 4)
    {
        double t = y / 340.0;
        int r = 25 + (int)(115 * t);
        int g = 115 + (int)(95 * t);
        int b = 235 + (int)(15 * t);
        box(0, y, 1200, y + 4, COL(r, g, b));
    }
}

void drawSun()
{
    int cx = 414, cy = 72;
    ell(cx, cy, 64, 64, COL(95,175,248));

    for (int i = 0; i < 16; i++)
    {
        double a  = i * 22.5 * PI / 180.0;
        double a1 = a - 0.14, a2 = a + 0.14;
        int tipR = (i % 2 == 0) ? 80 : 62;
        tri(cx + (int)(42 * cos(a1)), cy + (int)(42 * sin(a1)),
            cx + (int)(tipR * cos(a)), cy + (int)(tipR * sin(a)),
            cx + (int)(42 * cos(a2)), cy + (int)(42 * sin(a2)),
            COL(255,190,30));
    }
    ell(cx, cy, 45, 45, COL(255,165,20));
    ell(cx, cy, 41, 41, COL(255,210,30));
    ell(cx, cy, 33, 33, COL(255,232,70));
    ell(cx - 11, cy - 13, 14, 10, COL(255,250,170));
}

void drawCloud(int x, int y, double s)
{
    int dx[6] = {-60, -38, -12, 18, 46, 66};
    int dy[6] = {  6,  -8, -18, -14, -2,  8};
    int rr[6] = { 16,  22,  28,  26, 22, 15};

    for (int i = 0; i < 6; i++)
        ell(x + (int)(dx[i] * s), y + (int)((dy[i] + 6) * s), (int)(rr[i] * s), (int)(rr[i] * s), COL(190,215,240));
    ell(x, y + (int)(14 * s), (int)(78 * s), (int)(14 * s), COL(190,215,240));

    for (int i = 0; i < 6; i++)
        ell(x + (int)(dx[i] * s), y + (int)(dy[i] * s), (int)(rr[i] * s), (int)(rr[i] * s), COL(255,255,255));
    ell(x, y + (int)(9 * s), (int)(74 * s), (int)(12 * s), COL(255,255,255));
}


void mountain(int px, int py, int lx, int rx, int by, int light, int dark, int hi)
{
    int sx1 = lx + (px - lx) / 2, sy1 = (by + py) / 2 + 10;
    int sx2 = px + (rx - px) / 2, sy2 = (by + py) / 2 + 12;
    int fx  = px + (rx - px) / 8;

    int pl[8] = {lx, by, sx1, sy1, px, py, fx, by};
    poly(4, pl, light);
    int pd[8] = {px, py, sx2, sy2, rx, by, fx, by};
    poly(4, pd, dark);

    tri(px, py, px - (px - lx) / 6, py + (by - py) / 4, px + 4, py + (by - py) / 3, hi);
    thickLine(px, py, fx, by, 1, dark);
}

void drawMountains()
{
    mountain(707, 154, 560, 830, 290, COL(100,152,222), COL(70,115,192), COL(160,198,242));
    mountain(495, 182, 385, 620, 290, COL(95,148,220),  COL(68,112,190), COL(160,198,242));
    mountain(592, 215, 455, 740, 300, COL(85,140,215),  COL(55,100,182), COL(140,185,240));
    mountain(431, 248, 290, 585, 305, COL(75,130,208),  COL(50,95,178),  COL(130,175,235));
    mountain(105, 268,   0, 250, 310, COL(80,135,205),  COL(60,110,185), COL(130,175,235));
    mountain(215, 272, 100, 340, 310, COL(70,125,205),  COL(50,100,180), COL(120,170,235));
    mountain(800, 262, 700, 910, 305, COL(95,150,215),  COL(70,120,195), COL(150,195,240));
}

void drawRoundTree(int x, int y, int r)
{
    box(x - 3, y + r / 2, x + 3, y + r + 14, COL(95,60,30));
    ell(x, y, r, (int)(r * 0.9), COL(35,120,35));
    ell(x - r / 3, y - r / 5, r * 6 / 10, r * 6 / 10, COL(60,160,40));
    ell(x - r / 4, y - r / 2, r / 3, r / 3, COL(110,195,50));
    ell(x + r / 2, y - r / 6, r / 3, r / 3, COL(80,175,45));
}

void drawHills()
{
    ell(120, 330, 260, 50, COL(45,135,150));
    ell(520, 325, 320, 40, COL(40,125,140));
    ell(850, 318, 280, 40, COL(50,140,110));
    ell(1080, 330, 200, 50, COL(45,130,100));

    ell(100, 338, 240, 38, COL(50,150,60));
    ell(480, 340, 320, 34, COL(45,140,55));
    ell(820, 338, 250, 36, COL(55,155,60));


    drawRoundTree(43, 287, 30);
    drawRoundTree(201, 309, 28);
    drawRoundTree(395, 301, 32);
    drawRoundTree(517, 309, 28);
    drawRoundTree(596, 301, 30);
    drawRoundTree(718, 273, 36);
}

void drawGrass()
{
    for (int y = 322; y < 700; y += 6)
    {
        double t = (y - 322) / 378.0;
        box(0, y, 1200, y + 6, COL(75 + (int)(45 * t), 170 + (int)(35 * t), 35));
    }

    ell(930, 470, 260, 40, COL(125,205,45));
    ell(300, 410, 220, 22, COL(95,185,45));
    ell(1000, 430, 200, 20, COL(115,200,45));
}

void drawFence()
{
    box(0, 346, 650, 351, COL(150,90,40));
    box(0, 364, 650, 369, COL(150,90,40));


    for (int x = 0; x <= 650; x += 15)
    {
        int p[10] = {x, 347, x + 5, 337, x + 10, 347, x + 10, 378, x, 378};
        polyO(5, p, COL(205,135,65), COL(140,80,35));
        thickLine(x + 3, 349, x + 3, 376, 1, COL(228,160,85));
    }
}

void drawBush(int x, int y, int r, int dk, int md, int lt)
{
    ell(x, y, (int)(r * 1.3), r, dk);
    drawLeafCluster(x - r * 6 / 10, y - r / 5, r * 6 / 10, md);
    drawLeafCluster(x + r * 6 / 10, y - r / 5, r * 6 / 10, md);
    drawLeafCluster(x, y - r / 2, r * 7 / 10, md);
    drawLeafCluster(x - r * 3 / 10, y - r * 7 / 10, r * 4 / 10, lt);
    drawLeafCluster(x + r * 4 / 10, y - r * 5 / 10, r * 35 / 100, lt);
}

void drawFenceBushes()
{
    drawBush(194, 360, 24, G_DARK, G_MID, G_LIGHT);
    drawBush(474, 366, 24, G_DARK, G_MID, G_LIGHT);
    drawBush(560, 374, 20, G_DARK, G_MID, G_LIGHT);
    drawBush(632, 374, 24, G_DARK, G_MID, G_LIGHT);
}

void drawGrassTuft(int x, int y, int h)
{
    for (int k = -2; k <= 2; k++)
    {
        int c = (k % 2 == 0) ? COL(35,135,35) : COL(70,175,40);
        tri(x + k * 3, y, x + k * 3 + 5, y, x + k * 8, y - h + abs(k) * 6, c);
    }
}

void drawTree()
{
    int trunk[22] = {85,388, 112,365, 122,310, 126,250, 128,215, 160,215,
                     165,250, 172,310, 185,360, 212,388, 150,376};
    polyO(11, trunk, BROWN, BROWN_D);
    int light[12] = {118,372, 128,300, 134,240, 142,240, 138,300, 132,372};
    poly(6, light, COL(175,115,55));
    thickLine(150, 300, 152, 360, 1, BROWN_D);
    thickLine(142, 270, 146, 320, 1, BROWN_D);
    thickLine(122, 340, 126, 378, 1, BROWN_D);

    srand(11);
    ell(195, 128, 200, 122, COL(30,105,30));
    scatter(90, 195, 128, 190, 112, 24, COL(35,120,30));
    scatter(80, 200, 120, 175, 100, 20, COL(60,155,30));
    scatter(70, 205, 105, 160,  85, 16, COL(105,195,35));
    scatter(55, 215,  90, 140,  65, 11, COL(165,225,50));
    scatter(30, 225,  80, 110,  45,  8, COL(200,240,90));

    thickLine(135, 265,  70, 225, 3, COL(125,75,30));
    thickLine(130, 288,  40, 262, 3, COL(125,75,30));
    thickLine(158, 262, 235, 215, 3, COL(125,75,30));
    thickLine(162, 285, 250, 262, 3, COL(125,75,30));
    thickLine(145, 250, 150, 200, 3, COL(125,75,30));
}

void drawEvergreen(int x)
{
    box(x - 6, 290, x + 6, 335, COL(95,60,30));
    for (int k = 0; k < 7; k++)
    {
        int ty = 22 + k * 36;
        int by = ty + 58;
        int hw = 16 + k * 16;
        tri(x, ty, x - hw, by, x + hw, by, COL(22,95,45));
        tri(x, ty, x - hw, by, x - hw / 4, by, COL(45,135,60));
        for (int j = -2; j <= 2; j++)
            tri(x + j * hw / 3 - 8, by - 2, x + j * hw / 3 + 8, by - 2,
                x + j * hw / 3, by + 8, COL(22,95,45));
    }
}

void drawArchWindow(int x1, int y1, int x2, int y2)
{
    int w = x2 - x1, cx = (x1 + x2) / 2;
    ell(cx, y1 + w / 2, w / 2 + 3, w / 2 + 3, COL(165,95,45));
    box(x1 - 3, y1 + w / 2, x2 + 3, y2, COL(165,95,45));
    ell(cx, y1 + w / 2, w / 2 - 1, w / 2 - 1, COL(70,150,230));
    box(x1 + 1, y1 + w / 2, x2 - 1, y2 - 3, COL(70,150,230));
    tri(x1 + 4, y2 - 5, x1 + 16, y2 - 5, x1 + 4, y2 - 22, COL(170,215,250));
    thickLine(cx, y1 + 2, cx, y2 - 3, 1, COL(165,95,45));
    thickLine(x1, y1 + w / 2 + 6, x2, y1 + w / 2 + 6, 1, COL(165,95,45));
    box(x1 - 6, y2, x2 + 6, y2 + 5, COL(140,80,35));
}

void drawRectWindow(int x1, int y1, int x2, int y2)
{
    box(x1 - 3, y1 - 3, x2 + 3, y2 + 3, COL(165,95,45));
    box(x1, y1, x2, y2, COL(70,150,230));
    tri(x1 + 2, y2 - 2, x1 + 14, y2 - 2, x1 + 2, y2 - 18, COL(170,215,250));
    int cx = (x1 + x2) / 2, cy = (y1 + y2) / 2;
    thickLine(cx, y1, cx, y2, 3, COL(165,95,45));
    thickLine(x1, cy, x2, cy, 3, COL(165,95,45));
    box(x1 - 6, y2 + 3, x2 + 6, y2 + 8, COL(140,80,35));
}

void drawVines()
{
    for (int a = 0; a <= 180; a += 12)
    {
        double r = a * PI / 180.0;
        int x = 910 + (int)(46 * cos(r)), y = 318 - (int)(46 * sin(r));
        ell(x, y, 4, 4, G_MID);
        if ((a / 12) % 2 == 0) ell(x, y - 2, 3, 3, COL(255,120,170));
    }
    for (int y = 318; y <= 385; y += 9)
    {
        ell(864, y, 4, 4, G_MID);
        ell(956, y, 4, 4, G_MID);
        if (((y - 318) / 9) % 2 == 0)
        {
            ell(861, y, 3, 3, COL(255,120,170));
            ell(959, y, 3, 3, COL(255,150,190));
        }
    }
}

void drawPot(int x, int y, int flowerCol)
{
    int p[8] = {x - 13, y, x + 13, y, x + 9, y + 24, x - 9, y + 24};
    polyO(4, p, COL(205,105,50), COL(140,65,25));
    box(x - 15, y - 3, x + 15, y + 3, COL(225,125,60));
    ell(x, y - 8, 16, 11, G_MID);
    ell(x - 7, y - 12, 4, 4, flowerCol);
    ell(x + 2, y - 15, 4, 4, flowerCol);
    ell(x + 9, y - 10, 4, 4, COL(255,255,255));
    ell(x - 1, y - 8, 3, 3, flowerCol);
}

void drawHouseBushes()
{
    drawBush(715, 300, 46, G_DARK, G_MID, G_LIGHT);
    drawBush(705, 255, 34, G_DARK, G_MID, G_LIGHT);
    drawBush(735, 335, 36, G_DARK, G_MID, G_LIGHT);
    drawBush(735, 270, 28, G_DARK, G_MID, G_LIGHT);
}

void drawHouse()
{
    box(765, 270, 1117, 392, COL(245,225,170));
    box(765, 376, 1117, 392, COL(185,165,135));
    for (int x = 770; x < 1117; x += 36)
        thickLine(x, 376, x, 392, 1, COL(140,120,95));
    for (int y = 292; y < 376; y += 18)
        boxO(765, y, 779, y + 14, COL(225,205,155), COL(180,160,120));
    for (int y = 292; y < 376; y += 18)
        boxO(1103, y, 1117, y + 14, COL(225,205,155), COL(180,160,120));

    int gable[6] = {872, 133, 752, 278, 965, 278};
    poly(3, gable, COL(250,232,185));
    box(768, 250, 962, 259, COL(165,95,45));
    drawRectWindow(869, 192, 890, 219);

    int roof[8] = {872, 133, 1060, 215, 1152, 283, 960, 278};
    polyO(4, roof, COL(214,80,30), COL(150,55,20));

    for (int r = 1; r <= 9; r++)
    {
        double t  = r / 10.0, t2 = (r + 1) / 10.0;
        int lx  = 872 + (int)((960 - 872) * t),   ly  = 133 + (int)((278 - 133) * t);
        int rx  = 1060 + (int)((1152 - 1060) * t), ry  = 215 + (int)((283 - 215) * t);
        int lx2 = 872 + (int)((960 - 872) * t2),  ly2 = 133 + (int)((278 - 133) * t2);
        int rx2 = 1060 + (int)((1152 - 1060) * t2), ry2 = 215 + (int)((283 - 215) * t2);
        thickLine(lx, ly, rx, ry, 1, COL(160,55,20));
        for (int j = 0; j < 12; j++)
        {
            double s = (j + (r % 2) * 0.5) / 12.0;
            int ax = lx + (int)((rx - lx) * s),   ay = ly + (int)((ry - ly) * s);
            int bx = lx2 + (int)((rx2 - lx2) * s), by = ly2 + (int)((ry2 - ly2) * s);
            thickLine(ax, ay, bx, by, 1, COL(170,60,22));
        }
    }
    int ridge[8] = {872, 133, 1060, 215, 1052, 224, 876, 147};
    poly(4, ridge, COL(235,115,50));
    thickLine(738, 283, 872, 131, 3, COL(230,100,38));
    thickLine(741, 285, 875, 134, 1, COL(150,55,20));
    thickLine(960, 279, 1152, 284, 3, COL(150,55,20));

    boxO(991, 118, 1045, 215, COL(195,175,145), COL(120,100,75));
    for (int y = 130; y < 215; y += 12)
        thickLine(991, y, 1045, y, 1, COL(140,120,95));
    for (int y = 118; y < 215; y += 24)
    {
        thickLine(1005, y, 1005, y + 12, 1, COL(140,120,95));
        thickLine(1030, y + 12, 1030, y + 24, 1, COL(140,120,95));
    }
    boxO(985, 110, 1051, 121, COL(175,95,50), COL(110,55,25));

    drawArchWindow(790, 287, 836, 345);
    drawRectWindow(1030, 298, 1073, 348);

    ell(910, 318, 38, 38, COL(225,205,160));
    box(872, 318, 948, 392, COL(225,205,160));
    ell(910, 318, 34, 34, COL(110,60,25));
    box(876, 318, 944, 392, COL(110,60,25));
    ell(910, 320, 29, 29, COL(160,100,50));
    box(881, 320, 939, 392, COL(160,100,50));
    thickLine(895, 300, 895, 391, 1, COL(110,60,25));
    thickLine(910, 291, 910, 391, 1, COL(110,60,25));
    thickLine(925, 300, 925, 391, 1, COL(110,60,25));
    ell(900, 355, 3, 3, COL(240,190,50));
    ell(920, 355, 3, 3, COL(240,190,50));

    thickLine(906, 247, 906, 255, 1, COL(60,40,20));
    boxO(901, 255, 911, 268, COL(255,205,70), COL(60,40,20));

    boxO(858, 391, 958, 400, COL(175,175,180), COL(110,110,115));
    boxO(850, 400, 966, 412, COL(150,150,155), COL(100,100,105));

    drawVines();
    drawPot(842, 390, COL(255,110,170));
    drawPot(976, 394, COL(190,90,220));
}

void drawPond()
{
    ell(330, 515, 300, 80, COL(25,85,170));
    ell(540, 522, 110, 55, COL(25,85,170));
    ell(90,  500,  80, 40, COL(25,85,170));
    ell(300, 512, 270, 68, COL(40,130,220));
    ell(540, 520,  90, 42, COL(45,135,225));
    ell(330, 506, 200, 50, COL(70,165,238));
    ell(250, 526, 120, 30, COL(120,205,250));
    ell(440, 540, 100, 22, COL(100,190,245));

    setcolor(COL(170,220,252));
    for (int i = 0; i < 9; i++)
        line(80 + i * 60, 470 + (i % 3) * 40, 120 + i * 60, 470 + (i % 3) * 40);
}

double archY(double x)
{
    double t = (x - 346.0) / 158.0;
    return 412.0 - 58.0 * (1.0 - t * t);
}

void drawBridge()
{
    const int N = 21;
    int tx[N], ty[N];
    for (int i = 0; i < N; i++)
    {
        tx[i] = 188 + i * 316 / 20;
        ty[i] = (int)archY(tx[i]);
    }

    thickLine(252, 380, 252, 470, 3, COL(95,50,20));
    thickLine(256, 380, 256, 470, 3, COL(120,65,25));
    thickLine(440, 372, 440, 462, 3, COL(95,50,20));
    thickLine(444, 372, 444, 462, 3, COL(120,65,25));

    int under[84];
    for (int i = 0; i < N; i++)
    {
        under[2 * i] = tx[i];
        under[2 * i + 1] = ty[i] + 4;
        under[2 * (N + i)] = tx[N - 1 - i];
        under[2 * (N + i) + 1] = ty[N - 1 - i] + 30;
    }
    polyO(2 * N, under, COL(105,58,22), COL(70,35,12));

    int deck[84];
    for (int i = 0; i < N; i++)
    {
        deck[2 * i] = tx[i];
        deck[2 * i + 1] = ty[i];
        deck[2 * (N + i)] = tx[N - 1 - i];
        deck[2 * (N + i) + 1] = ty[N - 1 - i] + 14;
    }
    polyO(2 * N, deck, COL(190,115,52), COL(110,60,25));
    for (int i = 0; i < N; i++)
        thickLine(tx[i], ty[i], tx[i], ty[i] + 14, 1, COL(120,68,28));

    for (int i = 0; i < N - 1; i++)
    {
        thickLine(tx[i], ty[i] - 52, tx[i + 1], ty[i + 1] - 52, 3, COL(165,95,40));
        thickLine(tx[i], ty[i] - 30, tx[i + 1], ty[i + 1] - 30, 3, COL(150,85,35));
        thickLine(tx[i], ty[i] - 53, tx[i + 1], ty[i + 1] - 53, 1, COL(205,135,60));
    }
    for (int i = 0; i < N; i += 2)
    {
        thickLine(tx[i], ty[i] + 2, tx[i], ty[i] - 54, 3, COL(150,85,35));
        ell(tx[i], ty[i] - 57, 4, 4, COL(205,135,60));
    }
}

void drawLilyPad(int x, int y, int rx)
{
    int ry = rx * 3 / 10 + 1;
    ell(x, y, rx, ry, COL(35,125,50));
    ell(x, y - 1, rx - 3, ry - 1, COL(95,185,65));
    tri(x, y, x + rx, y - 2, x + rx, y + 3, COL(70,165,238));
}

void drawLotus(int x, int y, int s)
{
    double back[5] = {-160, -125, -90, -55, -20};
    for (int i = 0; i < 5; i++)
        petal(x, y, back[i], s, s / 3 + 1, COL(240,130,170), COL(200,80,130));
    double front[4] = {-130, -105, -75, -50};
    for (int i = 0; i < 4; i++)
        petal(x, y, front[i], s * 8 / 10, s / 3 + 1, COL(255,170,200), COL(215,100,150));
    petal(x, y, -90, s * 6 / 10, s / 4 + 1, COL(255,210,228), COL(220,120,160));
    ell(x, y - 2, s / 5 + 1, s / 6 + 1, COL(255,220,60));
}

void drawPondDetails()
{
    setcolor(COL(175,222,252));
    int rip[7][4] = { {108,512,50,10}, {169,500,40,8}, {316,512,45,10}, {370,515,55,12},
                      {273,540,60,13}, {485,550,70,14}, {294,492,35,7} };
    for (int i = 0; i < 7; i++)
    {
        ellipse(rip[i][0], rip[i][1], 0, 360, rip[i][2], rip[i][3]);
        ellipse(rip[i][0], rip[i][1], 0, 360, rip[i][2] + 12, rip[i][3] + 4);
    }

    drawLilyPad(108, 506, 38);
    drawLilyPad(169, 498, 28);
    drawLilyPad(316, 506, 36);
    drawLilyPad(370, 510, 40);
    drawLilyPad(273, 536, 52);
    drawLilyPad(485, 548, 52);
    drawLilyPad(294, 490, 25);

    drawLotus(108, 498, 22);
    drawLotus(169, 492, 12);
    drawLotus(294, 484, 14);
    drawLotus(340, 501, 18);
    drawLotus(273, 524, 28);
    drawLotus(485, 538, 28);
}

void drawRock(int x, int y, int rx, int ry, int body)
{
    ellO(x, y, rx, ry, body, COL(85,75,70));
    ell(x - rx / 4, y - ry / 3, rx / 2, ry * 4 / 10, COL(180,170,155));
    ell(x + rx / 5, y + ry / 2, rx * 6 / 10, ry / 4, COL(115,105,100));
}

void drawRocks()
{
    int rocks[12][4] = {
        {7,467,40,24}, {122,456,42,26}, {68,470,28,15}, {409,449,24,12},
        {477,463,26,13}, {520,465,30,16}, {560,474,34,18}, {621,463,45,28},
        {43,556,45,28}, {323,589,45,20}, {510,585,55,26}, {600,574,50,26} };
    for (int i = 0; i < 12; i++)
        drawRock(rocks[i][0], rocks[i][1], rocks[i][2], rocks[i][3],
                 (i % 2 == 0) ? COL(150,138,128) : COL(135,125,120));
}

void drawPathway()
{
    int stones[][4] = {
        {1270,590,45,7},  {1180,597,50,8},  {1095,607,45,8},  {1210,622,45,8},
        {1070,628,35,8},  {1135,630,40,9},  {1160,655,48,10}, {1265,660,45,10},
        {1200,682,45,11}, {1320,690,55,11}, {1240,710,48,11}, {1370,712,45,11},
        {1265,738,60,13}, {1150,745,45,11}, {1180,775,45,12}, {1320,795,55,13},
        {1090,800,40,11}, {1215,822,60,15}, {1050,832,60,15}, {1070,885,100,27},
        {900,888,65,20},  {790,925,50,14},  {960,930,80,22},  {1090,930,50,12} };
    int count = sizeof(stones) / sizeof(stones[0]);

    for (int i = 0; i < count; i++)
    {
        int x  = (int)(stones[i][0] * 0.7177);
        int y  = (int)(stones[i][1] * 0.7177);
        int rx = (int)(stones[i][2] * 0.7177);
        int ry = (int)(stones[i][3] * 0.7177);
        ell(x, y + 3, rx + 3, ry + 2, COL(165,120,70));
        ellO(x, y, rx, ry, COL(232,198,148), COL(190,150,100));
        ell(x - rx / 5, y - ry / 4, rx * 6 / 10, ry / 2, COL(245,222,175));
    }
}

void drawFlower(int x, int y, int r, int n, int petalCol, int centerCol, int stem)
{
    if (stem > 0)
    {
        thickLine(x, y, x, y + stem, 1, COL(30,120,30));
        ell(x - 6, y + stem / 2, 6, 3, G_MID);
        ell(x + 6, y + stem / 2 + 4, 6, 3, G_MID);
    }
    int pr = r / 2 + 1;
    for (int i = 0; i < n; i++)
    {
        double a = i * 2 * PI / n;
        ell(x + (int)(r * 0.6 * cos(a)), y + (int)(r * 0.6 * sin(a)), pr, pr, petalCol);
    }
    ell(x, y, r / 3 + 1, r / 3 + 1, centerCol);
}

void drawBigFlower(int x, int y, int len, int n, int wid, int fill, int line, int cen, int cenR)
{
    for (int i = 0; i < n; i++)
        petal(x, y, i * 360.0 / n, len, wid, fill, line);
    ell(x, y, cenR, cenR, cen);
    ell(x, y, cenR / 2, cenR / 2, COL(255,200,40));
}

void drawRose(int x, int y, int r)
{
    ellO(x, y, r, r, COL(205,25,40), COL(120,10,25));
    ell(x, y, r * 7 / 10, r * 7 / 10, COL(228,45,60));
    ell(x + 1, y - 1, r * 4 / 10, r * 4 / 10, COL(175,15,35));
    setcolor(COL(120,10,25));
    arc(x, y, 30, 250, r * 55 / 100);
    arc(x, y, 200, 420, r * 3 / 10);
    ell(x - r, y + r, r / 2 + 2, 3, G_DARK);
}

void drawLavender(int x, int y, int h)
{
    thickLine(x, y, x, y - h, 1, COL(40,120,40));
    for (int i = 0; i < h / 7; i++)
    {
        int yy = y - h + i * 7;
        ell(x - 3, yy, 4, 5, COL(120,80,205));
        ell(x + 3, yy + 3, 4, 5, COL(145,100,225));
    }
}

void drawWildPatch(int x, int y, int w, int h, int n, int c1, int c2)
{
    for (int i = 0; i < n; i++)
    {
        int px = x + rand() % w, py = y + rand() % h;
        if (i % 4 == 0) drawGrassTuft(px, py + 8, 18);
        drawFlower(px, py, 4, 5, (i % 2 == 0) ? c1 : c2, COL(255,215,50), 0);
    }
}

void drawFlowerBed(int x, int y, int w, int h, int n)
{
    int pal[7] = {COL(255,110,170), COL(235,60,140), COL(255,255,255), COL(255,215,40),
                  COL(255,140,40),  COL(170,90,220), COL(230,40,50)};
    for (int i = 0; i < n; i++)
        ell(x + rand() % w, y + h / 2 + rand() % (h / 2 + 1), 9, 5, G_MID);
    for (int i = 0; i < n; i++)
    {
        int px = x + rand() % w, py = y + rand() % h;
        drawFlower(px, py, 5 + rand() % 4, 5, pal[rand() % 7], COL(255,215,50), 0);
    }
}

void drawBeds()
{
    drawBush(718, 358, 38, G_DARK, G_MID, G_LIGHT);
    drawFlowerBed(680, 335, 80, 50, 22);
    drawBush(780, 392, 22, G_DARK, G_MID, G_LIGHT);
    drawFlowerBed(745, 380, 85, 30, 10);
    drawBush(1030, 392, 26, G_DARK, G_MID, G_LIGHT);
    drawWildPatch(1000, 372, 100, 36, 20, COL(255,255,255), COL(255,120,180));
    drawBush(1150, 382, 36, G_DARK, G_MID, G_LIGHT);
    drawBush(1185, 395, 28, G_DARK, G_MID, G_LIGHT);
    drawFlowerBed(1110, 350, 90, 55, 26);

    for (int i = 0; i < 9; i++)
        drawGrassTuft(548 + i * 11, 462, 40 + (i % 3) * 14);
    drawWildPatch(545, 400, 100, 60, 24, COL(255,255,255), COL(255,255,255));

    drawBush(685, 545, 42, COL(60,130,30), COL(110,170,30), COL(170,210,40));
    drawWildPatch(645, 505, 90, 70, 20, COL(255,220,30), COL(255,240,90));

    drawBush(55, 425, 38, G_DARK, G_MID, G_LIGHT);
    drawFlowerBed(5, 380, 100, 60, 25);

    drawBush(1120, 505, 45, G_DARK, G_MID, G_LIGHT);
    drawFlowerBed(1060, 440, 130, 90, 28);
}

void drawForegroundFlowers()
{
    drawGrassTuft(1010, 445, 28);
    drawGrassTuft(1085, 460, 30);
    drawGrassTuft(1020, 540, 30);
    drawGrassTuft(1055, 570, 32);
    drawGrassTuft(800, 645, 28);
    drawGrassTuft(540, 692, 45);
    drawGrassTuft(30, 500, 55);
    drawGrassTuft(612, 520, 30);

    petal(20, 700, -60, 85, 20, COL(40,150,50), COL(20,95,30));
    petal(110, 700, -75, 75, 18, COL(50,160,55), COL(20,95,30));
    petal(190, 700, -110, 70, 18, COL(40,150,50), COL(20,95,30));
    petal(300, 700, -70, 60, 15, COL(60,165,55), COL(20,95,30));
    petal(600, 700, -100, 65, 16, COL(40,150,50), COL(20,95,30));

    drawBigFlower(40, 560, 36, 7, 13, COL(250,125,175), COL(200,70,120), COL(255,160,40), 11);
    drawBigFlower(90, 598, 40, 8, 14, COL(250,120,170), COL(200,70,120), COL(255,160,40), 12);

    drawFlower(201, 581, 14, 10, COL(255,255,255), COL(255,200,40), 45);
    drawFlower(158, 603, 14, 10, COL(255,255,255), COL(255,200,40), 40);
    drawFlower(269, 650, 14, 10, COL(255,255,255), COL(255,200,40), 30);
    drawFlower(250, 612, 10,  8, COL(255,255,255), COL(255,200,40), 35);

    drawFlower(327, 570, 9, 6, COL(255,215,30), COL(230,120,20), 40);
    drawFlower(267, 617, 8, 6, COL(255,215,30), COL(230,120,20), 30);

    drawRose(345, 600, 20);
    drawRose(391, 614, 18);
    drawRose(355, 650, 22);
    drawRose(316, 617, 14);

    drawFlower(441, 612, 10, 5, COL(150,90,230), COL(255,230,80), 30);
    drawFlower(459, 635, 10, 5, COL(150,90,230), COL(255,230,80), 25);
    drawFlower(477, 616,  9, 5, COL(110,70,210), COL(255,230,80), 28);

    drawFlower(894, 627, 12, 6, COL(255,110,170), COL(255,230,80), 40);
    drawFlower(868, 660, 10, 6, COL(255,110,170), COL(255,230,80), 30);
    drawFlower(951, 626, 12, 6, COL(255,110,170), COL(255,230,80), 40);
    drawFlower(976, 657, 11, 6, COL(255,130,180), COL(255,230,80), 30);

    drawFlower(1113, 459, 11, 8, COL(255,135,40), COL(255,230,80), 0);
    drawFlower(1148, 449, 11, 8, COL(255,100,90), COL(255,230,80), 0);
    drawFlower(1134, 477, 11, 8, COL(255,110,130), COL(255,230,80), 0);
    drawFlower(1091, 502, 10, 8, COL(255,150,50), COL(255,230,80), 0);
    drawFlower(1163, 517, 10, 6, COL(190,90,220), COL(255,230,80), 0);

    drawLavender(1175, 632, 90);
    drawLavender(1150, 640, 60);
    drawLavender(1128, 650, 45);
    drawLavender(1000, 632, 50);
    drawLavender(1018, 640, 36);

    thickLine(1055, 620, 1055, 690, 3, COL(35,125,35));
    thickLine(1034, 590, 1034, 650, 3, COL(35,125,35));
    thickLine(1098, 570, 1098, 640, 3, COL(35,125,35));
    drawBigFlower(1034, 574, 22, 12, 7, COL(255,205,25), COL(200,130,10), COL(110,60,20), 9);
    drawBigFlower(1098, 556, 20, 12, 6, COL(255,205,25), COL(200,130,10), COL(110,60,20), 8);
    drawBigFlower(1055, 606, 42, 16, 9, COL(255,200,20), COL(200,130,10), COL(110,60,20), 20);
}


void drawGarden()
{
    srand(5);

    drawSky();
    drawSun();
    drawCloud(700, 55, 1.0);
    drawCloud(905, 100, 1.2);
    drawCloud(538, 130, 1.1);
    drawCloud(1090, 50, 0.6);
    drawCloud(420, 195, 0.4);
    drawMountains();
    drawHills();
    drawGrass();
    drawFence();
    drawFenceBushes();
    drawEvergreen(1131);
    drawTree();
    drawHouseBushes();
    drawHouse();
    drawPond();
    drawBridge();
    drawPondDetails();
    drawRocks();
    drawPathway();
    drawBeds();
    drawForegroundFlowers();
}

int main()
{
    initwindow(1200, 700, "Enchanted Garden");
    drawGarden();
    getch();
    closegraph();
    return 0;
}
