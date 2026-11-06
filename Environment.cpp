#include "Environment.h"
#include "Renderer.h"
#include "UI.h"
#include "Player.h"
#include "Enemy.h"
#include "PlayerCombat.h"
#include "Collectibles.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr float LANE_TOP_CENTER = 0.43f;
    constexpr float LANE_SPACING = 0.33f;
    constexpr float LANE_HEIGHT = 0.28f;

    void drawLaneAccent(GLint transformLocation, GLint colorLocation,
                        float x, float y, float width, float height,
                        float red, float green, float blue)
    {
        glUniform3f(colorLocation, red, green, blue);
        drawPictureTinted(transformLocation, "ui-fill", x, y, width, height);
    }

    void drawFoodBase(GLint t, GLint c, const Environment& environment)
    {
        const FoodPicture foods[] = { FOOD_BREAD, FOOD_APPLE, FOOD_CHEESE, FOOD_BREAD };
        float x = environment.getHomeBaseRightEdgeX() - 0.14f;
        for (int lane = 0; lane < Environment::LANE_COUNT; ++lane)
            drawFoodPlate(t, c, foods[lane], x, environment.getLaneCenterY(lane));
    }

}

void Environment::setViewSize(float halfWidth, float halfHeight)
{
    if (!std::isfinite(halfWidth) || !std::isfinite(halfHeight) ||
        halfWidth < 1.0f || halfHeight < 1.0f) return;

    float oldStart = getLaneStartX();
    float oldWidth = getLaneEndX() - oldStart;
    viewHalfWidth = halfWidth;
    viewHalfHeight = halfHeight;

    // Preserve progress along the counter when the window is resized.
    float scale = (getLaneEndX() - getLaneStartX()) / oldWidth;
    float offset = getLaneStartX() - oldStart * scale;
    for (Player& player : getPlayers()) {
        player.x = player.x * scale + offset;
    }
    resizeEnemyPositions(scale, offset);
    resizePlayerProjectiles(scale, offset);
    resizeCollectiblePositions(scale, offset);
    for (int lane = 0; lane < LANE_COUNT; ++lane) {
        if (hasPlacement[lane]) {
            placementX[lane] = placementX[lane] * scale + offset;
        }
    }
}

void Environment::update(float dt) { elapsed += std::max(0.0f, dt); }

void Environment::render(GLint transformLocation, GLint colorLocation,
                         int, int, int)
{
    float left = getLaneStartX();
    float width = getLaneEndX() - left;
    float center = left + width * 0.5f;
    int hoverLane = isGameplayRunning() ? getLaneAtPoint(mouseX, mouseY) : -1;

    for (int lane = 0; lane < LANE_COUNT; ++lane)
    {
        float y = getLaneCenterY(lane);
        if (selected[lane]) {
            glUniform3f(colorLocation, 1.0f, 0.90f, 0.65f);
        } else if (lane == hoverLane) {
            glUniform3f(colorLocation, 0.83f, 0.95f, 0.94f);
        } else if (lane % 2) {
            glUniform3f(colorLocation, 0.96f, 0.94f, 0.89f);
        } else {
            glUniform3f(colorLocation, 1.0f, 1.0f, 0.98f);
        }

        drawKitchenCounter(transformLocation, center, y, width, LANE_HEIGHT);
        drawLaneAccent(transformLocation, colorLocation, center, y - LANE_HEIGHT * 0.5f - 0.012f,
                       width, 0.007f, 0.72f, 0.60f, 0.43f);
        if (selected[lane] || lane == hoverLane)
            drawLaneAccent(transformLocation, colorLocation, left + 0.006f, y, 0.012f, LANE_HEIGHT,
                           selected[lane] ? 0.88f : 0.35f, selected[lane] ? 0.65f : 0.70f, 0.28f);
    }

    drawFoodBase(transformLocation, colorLocation, *this);
}

void Environment::handleMouseMove(float x, float y)
{
    mouseX = x;
    mouseY = y;
}

void Environment::handleMouseButton(int button, int action, float x, float y)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;
    if (action == GLFW_RELEASE)
    {
        leftButtonDown = false;
        return;
    }
    if (action != GLFW_PRESS || leftButtonDown)
        return;
    leftButtonDown = true;
    if (getGameState() != STATE_PLAYING)
        return;
    const int lane = getLaneAtPoint(x, y);

    if (lane < 0)
        return;

    // Defense cards use the last click position in each selected lane.
    placementX[lane] = x;
    hasPlacement[lane] = true;

    selected[lane] = !selected[lane];
}

int Environment::getLaneCount() const { return LANE_COUNT; }
float Environment::getLaneCenterY(int i) const
{
    return (i >= 0 && i < LANE_COUNT) ? LANE_TOP_CENTER - i * LANE_SPACING : 0.0f;
}
float Environment::getLaneTop(int i) const { return getLaneCenterY(i) + LANE_HEIGHT * 0.5f; }
float Environment::getLaneBottom(int i) const { return getLaneCenterY(i) - LANE_HEIGHT * 0.5f; }
float Environment::getLaneStartX() const { return -viewHalfWidth + 0.32f; }
float Environment::getLaneEndX() const { return viewHalfWidth - 0.08f; }
float Environment::getHomeBaseRightEdgeX() const { return getLaneStartX() - 0.02f; }

int Environment::getLaneAtPoint(float x, float y) const
{
    if (x < getLaneStartX() || x > getLaneEndX())
        return -1;
    for (int i = 0; i < LANE_COUNT; ++i)
    {
        if (y >= getLaneBottom(i) && y <= getLaneTop(i))
            return i;
    }
    return -1;
}

bool Environment::isLaneSelected(int i) const { return i >= 0 && i < LANE_COUNT && selected[i]; }
std::vector<int> Environment::getSelectedLanes() const
{
    std::vector<int> result;
    for (int i = 0; i < LANE_COUNT; ++i)
        if (selected[i])
            result.push_back(i);
    return result;
}
void Environment::clearSelection()
{
    for (bool &value : selected)
        value = false;
}

void Environment::reset()
{
    clearSelection();

    for (bool &value : hasPlacement)
    {
        value = false;
    }

    elapsed = 0.0f;
    leftButtonDown = false;
}

float Environment::getPlacementX(int lane) const
{
    if (lane >= 0 && lane < LANE_COUNT && hasPlacement[lane])
    {
        return placementX[lane];
    }

    return getLaneStartX() + 0.08f;
}
