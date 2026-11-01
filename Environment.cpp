#include "Environment.h"

#include "Renderer.h"
#include "UI.h"

#include <algorithm>
#include <cmath>

namespace {
// Battlefield geometry and colors are kept together for easy tuning.
constexpr float LANE_START_X = -0.68f;
constexpr float LANE_END_X = 0.92f;
constexpr float LANE_TOP_CENTER = 0.43f;
constexpr float LANE_SPACING = 0.33f;
constexpr float LANE_HEIGHT = 0.28f;
constexpr float HOME_RIGHT_EDGE = -0.70f;
constexpr float HOVER_COLOR[3] = {0.40f, 0.78f, 0.84f};
constexpr float SELECT_COLOR[3] = {0.92f, 0.67f, 0.22f};
constexpr bool PLACEHOLDER_TEST_MODE = true;

void color(GLint location, float r, float g, float b) { glUniform3f(location, r, g, b); }

void rect(GLint t, GLint c, float x, float y, float w, float h, float r, float g, float b, float angle = 0.0f) {
    color(c, r, g, b);
    drawRectangle(t, x, y, w, h, angle);
}

void circle(GLint t, GLint c, int first, int count, float x, float y, float radius, float r, float g, float b) {
    color(c, r, g, b);
    drawCircle(t, first, count, x, y, radius);
}

void triangle(GLint t, GLint c, int first, float x, float y, float w, float h, float r, float g, float b, float angle = 0.0f) {
    color(c, r, g, b);
    drawTriangle(t, first, x, y, w, h, angle);
}

void drawFoodBase(GLint t, GLint c, int firstCircle, int circleCount, int firstTriangle) {
    // A shaded platter, basket, bread loaf, apple, and cheese form a readable food target.
    rect(t, c, -0.84f, -0.14f, 0.28f, 1.22f, 0.43f, 0.25f, 0.14f);
    rect(t, c, -0.84f, -0.14f, 0.24f, 1.15f, 0.72f, 0.48f, 0.25f);
    rect(t, c, -0.84f, -0.60f, 0.30f, 0.08f, 0.42f, 0.24f, 0.12f);
    circle(t, c, firstCircle, circleCount, -0.84f, -0.12f, 0.12f, 0.92f, 0.78f, 0.52f);
    circle(t, c, firstCircle, circleCount, -0.86f, -0.10f, 0.085f, 0.88f, 0.62f, 0.25f);
    circle(t, c, firstCircle, circleCount, -0.79f, -0.08f, 0.047f, 0.75f, 0.12f, 0.10f);
    rect(t, c, -0.83f, -0.015f, 0.018f, 0.07f, 0.18f, 0.42f, 0.14f, -20.0f);
    triangle(t, c, firstTriangle, -0.85f, -0.23f, 0.16f, 0.13f, 1.0f, 0.82f, 0.28f);
    circle(t, c, firstCircle, circleCount, -0.87f, -0.22f, 0.012f, 0.78f, 0.47f, 0.10f);
    circle(t, c, firstCircle, circleCount, -0.82f, -0.20f, 0.012f, 0.78f, 0.47f, 0.10f);
}

void drawKitchenDetails(GLint t, GLint c, int firstCircle, int circleCount, int firstTriangle) {
    // Upper wall tile strip: cabinet, curtained window, sink with faucet.
    rect(t, c, 0.0f, 0.685f, 2.0f, 0.15f, 0.77f, 0.84f, 0.82f);
    for (int i = 0; i < 9; ++i) {
        rect(t, c, -0.90f + i * 0.225f, 0.685f, 0.008f, 0.15f, 0.58f, 0.70f, 0.68f);
    }
    rect(t, c, -0.51f, 0.687f, 0.39f, 0.12f, 0.55f, 0.32f, 0.19f);
    rect(t, c, -0.51f, 0.687f, 0.35f, 0.095f, 0.77f, 0.56f, 0.34f);
    rect(t, c, -0.51f, 0.687f, 0.018f, 0.09f, 0.47f, 0.27f, 0.17f);
    circle(t, c, firstCircle, circleCount, -0.51f, 0.687f, 0.009f, 0.93f, 0.83f, 0.58f);
    rect(t, c, 0.0f, 0.69f, 0.27f, 0.11f, 0.43f, 0.65f, 0.78f);
    rect(t, c, 0.0f, 0.69f, 0.23f, 0.085f, 0.67f, 0.85f, 0.92f);
    rect(t, c, -0.13f, 0.69f, 0.045f, 0.105f, 0.90f, 0.47f, 0.43f);
    rect(t, c, 0.13f, 0.69f, 0.045f, 0.105f, 0.90f, 0.47f, 0.43f);
    rect(t, c, 0.43f, 0.635f, 0.38f, 0.035f, 0.52f, 0.58f, 0.61f);
    rect(t, c, 0.43f, 0.655f, 0.035f, 0.07f, 0.60f, 0.65f, 0.65f);
    rect(t, c, 0.43f, 0.69f, 0.12f, 0.025f, 0.60f, 0.65f, 0.65f);
    circle(t, c, firstCircle, circleCount, 0.49f, 0.677f, 0.014f, 0.60f, 0.65f, 0.65f);

    // Lower counter ledge carries a toaster, cutting board and mug (all outside lanes).
    rect(t, c, 0.0f, -0.91f, 2.0f, 0.18f, 0.57f, 0.37f, 0.23f);
    rect(t, c, 0.0f, -0.82f, 2.0f, 0.025f, 0.77f, 0.58f, 0.35f);
    rect(t, c, -0.53f, -0.885f, 0.27f, 0.11f, 0.77f, 0.29f, 0.22f);
    rect(t, c, -0.53f, -0.87f, 0.20f, 0.065f, 0.88f, 0.43f, 0.29f);
    rect(t, c, -0.53f, -0.82f, 0.13f, 0.012f, 0.31f, 0.30f, 0.29f);
    rect(t, c, 0.0f, -0.89f, 0.32f, 0.11f, 0.63f, 0.36f, 0.19f);
    rect(t, c, 0.0f, -0.885f, 0.23f, 0.02f, 0.81f, 0.63f, 0.35f);
    rect(t, c, 0.51f, -0.88f, 0.12f, 0.12f, 0.93f, 0.85f, 0.69f);
    rect(t, c, 0.58f, -0.88f, 0.07f, 0.018f, 0.93f, 0.85f, 0.69f);
    circle(t, c, firstCircle, circleCount, 0.51f, -0.82f, 0.025f, 0.93f, 0.85f, 0.69f);

    // Salt and pepper shakers sit on the right end of the lower ledge.
    rect(t, c, 0.77f, -0.88f, 0.07f, 0.12f, 0.92f, 0.89f, 0.76f);
    rect(t, c, 0.77f, -0.81f, 0.045f, 0.025f, 0.75f, 0.70f, 0.51f);
    circle(t, c, firstCircle, circleCount, 0.77f, -0.85f, 0.006f, 0.35f, 0.30f, 0.25f);
    rect(t, c, 0.88f, -0.88f, 0.07f, 0.12f, 0.35f, 0.32f, 0.28f);
    rect(t, c, 0.88f, -0.81f, 0.045f, 0.025f, 0.73f, 0.67f, 0.49f);
    circle(t, c, firstCircle, circleCount, 0.88f, -0.85f, 0.006f, 0.92f, 0.88f, 0.73f);
    (void)firstTriangle;
}
}

void Environment::update(float dt) { elapsed += std::max(0.0f, dt); }

void Environment::render(GLint transformLocation, GLint colorLocation,
                         int circleVertexStart, int circleVertexCount, int triangleVertexStart) {
    // Countertop field and separation lines.
    rect(transformLocation, colorLocation, 0.0f, -0.08f, 2.0f, 1.40f, 0.72f, 0.66f, 0.55f);
    for (int i = 0; i < 4; ++i) {
        float y = getLaneCenterY(i);
        rect(transformLocation, colorLocation, (LANE_START_X + LANE_END_X) * 0.5f, y,
             LANE_END_X - LANE_START_X, LANE_HEIGHT + 0.025f, 0.46f, 0.38f, 0.29f);
        rect(transformLocation, colorLocation, (LANE_START_X + LANE_END_X) * 0.5f, y,
             LANE_END_X - LANE_START_X - 0.018f, LANE_HEIGHT, (i % 2) ? 0.89f : 0.94f,
             (i % 2) ? 0.82f : 0.88f, (i % 2) ? 0.69f : 0.77f);
    }

    int hoverLane = (getGameState() == STATE_PLAYING) ? getLaneAtPoint(mouseX, mouseY) : -1;
    for (int i = 0; i < LANE_COUNT; ++i) {
        if (selected[i]) {
            color(colorLocation, SELECT_COLOR[0], SELECT_COLOR[1], SELECT_COLOR[2]);
            drawRectangle(transformLocation, (LANE_START_X + LANE_END_X) * 0.5f, getLaneCenterY(i),
                          LANE_END_X - LANE_START_X, LANE_HEIGHT);
        } else if (i == hoverLane) {
            color(colorLocation, HOVER_COLOR[0], HOVER_COLOR[1], HOVER_COLOR[2]);
            drawRectangle(transformLocation, (LANE_START_X + LANE_END_X) * 0.5f, getLaneCenterY(i),
                          LANE_END_X - LANE_START_X, LANE_HEIGHT);
        }
    }

    drawFoodBase(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart);
    drawKitchenDetails(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart);

    if (PLACEHOLDER_TEST_MODE) {
        const int enemyLanes[] = {0, 1, 3};
        for (int i = 0; i < 3; ++i) {
            const int lane = enemyLanes[i];
            const float travel = std::fmod(elapsed * 0.10f + i * 0.28f, 0.46f);
            const float x = LANE_END_X - 0.12f - travel;
            const float y = getLaneCenterY(lane);
            drawMenuInsect(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart, x, y - 0.01f, 0.11f);
            drawHealthBar(transformLocation, colorLocation, x, y + 0.085f, 4, 5);
        }
    }
}

void Environment::handleMouseMove(float x, float y) { mouseX = x; mouseY = y; }

void Environment::handleMouseButton(int button, int action, float x, float y) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    if (action == GLFW_RELEASE) { leftButtonDown = false; return; }
    if (action != GLFW_PRESS || leftButtonDown) return;
    leftButtonDown = true;
    if (getGameState() != STATE_PLAYING) return;
    const int lane = getLaneAtPoint(x, y);
    if (lane >= 0) selected[lane] = !selected[lane];
}

int Environment::getLaneCount() const { return LANE_COUNT; }
float Environment::getLaneCenterY(int i) const { return (i >= 0 && i < LANE_COUNT) ? LANE_TOP_CENTER - i * LANE_SPACING : 0.0f; }
float Environment::getLaneTop(int i) const { return getLaneCenterY(i) + LANE_HEIGHT * 0.5f; }
float Environment::getLaneBottom(int i) const { return getLaneCenterY(i) - LANE_HEIGHT * 0.5f; }
float Environment::getLaneStartX() const { return LANE_START_X; }
float Environment::getLaneEndX() const { return LANE_END_X; }
float Environment::getHomeBaseRightEdgeX() const { return HOME_RIGHT_EDGE; }

int Environment::getLaneAtPoint(float x, float y) const {
    if (x < LANE_START_X || x > LANE_END_X) return -1;
    for (int i = 0; i < LANE_COUNT; ++i) {
        if (y >= getLaneBottom(i) && y <= getLaneTop(i)) return i;
    }
    return -1;
}

bool Environment::isLaneSelected(int i) const { return i >= 0 && i < LANE_COUNT && selected[i]; }
std::vector<int> Environment::getSelectedLanes() const {
    std::vector<int> result;
    for (int i = 0; i < LANE_COUNT; ++i) if (selected[i]) result.push_back(i);
    return result;
}
void Environment::clearSelection() { for (bool& value : selected) value = false; }
void Environment::reset() { clearSelection(); elapsed = 0.0f; leftButtonDown = false; }
