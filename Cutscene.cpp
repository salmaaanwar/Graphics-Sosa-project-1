#include "Cutscene.h"
#include "Renderer.h"
#include "TextRenderer.h"
#include "UI.h"
#include <cmath>
#include <string>

static float cutsceneTime = 0.0f;

void resetCutscene()
{
    cutsceneTime = 0.0f;
}

void updateCutscene(float deltaTime)
{
    if (deltaTime <= 0.0f || !std::isfinite(deltaTime)) return;
    cutsceneTime += deltaTime;
    if (cutsceneTime > CUTSCENE_DURATION) cutsceneTime = CUTSCENE_DURATION;
}

float getCutsceneTime()
{
    return cutsceneTime;
}

bool isCutsceneFinished()
{
    return cutsceneTime >= CUTSCENE_DURATION;
}

// Turn an interval into a number from 0 to 1, then interpolate two positions.
static float sceneProgress(float startTime, float endTime)
{
    return glm::clamp((cutsceneTime - startTime) / (endTime - startTime), 0.0f, 1.0f);
}

static void drawSceneText(GLint transformLocation, const std::string& text, float y, float cellSize)
{
    float width = (6.0f * static_cast<float>(text.length()) - 1.0f) * cellSize;
    drawText(transformLocation, text, -width * 0.5f, y + 3.5f * cellSize, cellSize);
}

static void drawKitchen(GLint transformLocation, GLint colorLocation,
    int circleVertexStart, int circleVertexCount)
{
    // Tile wall, countertop and a cabinet: composite scenery, like the UI icons.
    // The shared kitchen image fills the space around the animated stage.
    glUniform3f(colorLocation, 0.80f, 0.88f, 0.83f);
    drawRectangle(transformLocation, 0.0f, 0.12f, 1.88f, 0.92f);
    glUniform3f(colorLocation, 0.64f, 0.75f, 0.69f);
    for (int tile = 0; tile < 7; tile++)
    {
        drawRectangle(transformLocation, -0.90f + tile * 0.30f, 0.12f, 0.012f, 0.92f);
    }
    drawRectangle(transformLocation, 0.0f, 0.27f, 1.88f, 0.012f);
    drawRectangle(transformLocation, 0.0f, -0.03f, 1.88f, 0.012f);

    glUniform3f(colorLocation, 0.76f, 0.56f, 0.34f);
    drawRectangle(transformLocation, 0.0f, -0.36f, 1.88f, 0.42f);
    glUniform3f(colorLocation, 0.96f, 0.86f, 0.67f);
    drawRectangle(transformLocation, 0.0f, -0.18f, 1.88f, 0.07f);
    glUniform3f(colorLocation, 0.40f, 0.28f, 0.20f);
    drawRectangle(transformLocation, 0.0f, -0.51f, 1.88f, 0.025f);

    glUniform3f(colorLocation, 0.44f, 0.29f, 0.20f);
    drawRectangle(transformLocation, -0.65f, 0.44f, 0.48f, 0.22f);
    glUniform3f(colorLocation, 0.68f, 0.48f, 0.29f);
    drawRectangle(transformLocation, -0.65f, 0.44f, 0.43f, 0.17f);
    glUniform3f(colorLocation, 0.95f, 0.82f, 0.53f);
    drawRectangle(transformLocation, -0.49f, 0.43f, 0.018f, 0.07f);

    // A wall clock reinforces that this is the family's kitchen at dinner time.
    glUniform3f(colorLocation, 0.23f, 0.31f, 0.33f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, 0.65f, 0.43f, 0.125f);
    glUniform3f(colorLocation, 1.0f, 0.97f, 0.84f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, 0.65f, 0.43f, 0.105f);
    glUniform3f(colorLocation, 0.23f, 0.31f, 0.33f);
    drawRectangle(transformLocation, 0.65f, 0.465f, 0.012f, 0.07f);
    drawRectangle(transformLocation, 0.68f, 0.43f, 0.06f, 0.012f);

    // The plate of food is the home base that the cleaning tools must defend.
    glUniform3f(colorLocation, 0.58f, 0.40f, 0.26f);
    drawRectangle(transformLocation, -0.65f, -0.12f, 0.48f, 0.15f);
    glUniform3f(colorLocation, 0.95f, 0.98f, 1.0f);
    drawRectangle(transformLocation, -0.65f, -0.07f, 0.43f, 0.045f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, -0.83f, -0.07f, 0.025f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, -0.47f, -0.07f, 0.025f);
    glUniform3f(colorLocation, 0.34f, 0.61f, 0.23f);
    for (int leaf = 0; leaf < 4; leaf++)
    {
        drawCircle(transformLocation, circleVertexStart, circleVertexCount,
            -0.79f + leaf * 0.09f, -0.015f, 0.055f);
    }
    glUniform3f(colorLocation, 0.91f, 0.63f, 0.30f);
    drawRectangle(transformLocation, -0.67f, 0.015f, 0.20f, 0.08f, -10.0f);
    glUniform3f(colorLocation, 0.85f, 0.19f, 0.16f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, -0.77f, 0.005f, 0.035f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, -0.53f, 0.005f, 0.035f);

    // Small rising steam puffs keep the establishing shot animated as well.
    glUniform3f(colorLocation, 0.98f, 0.98f, 0.87f);
    for (int puff = 0; puff < 3; puff++)
    {
        float rise = std::fmod(cutsceneTime * 0.12f + puff * 0.06f, 0.18f);
        float sway = 0.012f * std::sin(cutsceneTime * 3.0f + puff);
        drawCircle(transformLocation, circleVertexStart, circleVertexCount,
            -0.75f + puff * 0.10f + sway, 0.10f + rise, 0.014f);
    }
}

static void drawSwatterGuard(GLint transformLocation, GLint colorLocation,
    int circleVertexStart, int circleVertexCount, int triangleVertexStart,
    float x, float y, float attack)
{
    // Reuse Member 3's swatter, then add a face, feet and animated arms.
    drawSwatterIcon(transformLocation, colorLocation, circleVertexStart,
        circleVertexCount, triangleVertexStart, x, y, 0.20f, false);
    glUniform3f(colorLocation, 0.98f, 0.98f, 0.94f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.032f, y + 0.09f, 0.023f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x + 0.032f, y + 0.09f, 0.023f);
    glUniform3f(colorLocation, 0.12f, 0.18f, 0.21f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.025f, y + 0.09f, 0.010f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x + 0.039f, y + 0.09f, 0.010f);
    drawRectangle(transformLocation, x, y + 0.035f, 0.04f, 0.012f);
    drawRectangle(transformLocation, x - 0.04f, y - 0.21f, 0.07f, 0.035f);
    drawRectangle(transformLocation, x + 0.04f, y - 0.21f, 0.07f, 0.035f);

    float armSwing = 12.0f * std::sin(cutsceneTime * 5.0f);
    glUniform3f(colorLocation, 0.93f, 0.29f, 0.32f);
    drawRectangle(transformLocation, x - 0.10f, y - 0.035f, 0.10f, 0.025f, 25.0f + armSwing);
    drawRectangle(transformLocation, x + 0.11f, y - 0.015f + attack * 0.06f,
        0.15f, 0.03f, -20.0f + attack * 65.0f);
}

static void drawCutsceneInsect(GLint transformLocation, GLint colorLocation,
    int circleVertexStart, int circleVertexCount, float x, float y, float scale)
{
    // A separate small placeholder lets its wings flap without changing UI.cpp.
    float flap = std::sin(cutsceneTime * 30.0f);
    glUniform3f(colorLocation, 0.86f, 0.97f, 1.0f);
    drawRectangle(transformLocation, x - 0.05f * scale, y + 0.50f * scale,
        0.80f * scale, 0.23f * scale, 25.0f + flap * 35.0f);
    drawRectangle(transformLocation, x + 0.35f * scale, y + 0.48f * scale,
        0.70f * scale, 0.23f * scale, -25.0f - flap * 35.0f);
    glUniform3f(colorLocation, 0.19f, 0.27f, 0.23f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x + 0.20f * scale, y, 0.42f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.28f * scale, y + 0.06f * scale, 0.30f * scale);
    glUniform3f(colorLocation, 0.54f, 0.68f, 0.31f);
    drawRectangle(transformLocation, x + 0.25f * scale, y, 0.09f * scale, 0.72f * scale, 15.0f);
    glUniform3f(colorLocation, 0.95f, 0.24f, 0.19f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.42f * scale, y + 0.12f * scale, 0.15f * scale);
    glUniform3f(colorLocation, 1.0f, 0.94f, 0.75f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.47f * scale, y + 0.16f * scale, 0.045f * scale);
    glUniform3f(colorLocation, 0.12f, 0.18f, 0.17f);
    for (int leg = 0; leg < 3; leg++)
    {
        drawRectangle(transformLocation, x + (-0.22f + leg * 0.25f) * scale,
            y - 0.37f * scale, 0.06f * scale, 0.38f * scale,
            -25.0f + leg * 25.0f + flap * 8.0f);
    }
    drawRectangle(transformLocation, x - 0.32f * scale, y + 0.43f * scale,
        0.05f * scale, 0.30f * scale, 25.0f);
}

void drawCutscene(GLint transformLocation, GLint colorLocation,
    int circleVertexStart, int circleVertexCount, int triangleVertexStart)
{
    drawKitchen(transformLocation, colorLocation, circleVertexStart, circleVertexCount);

    // Five short story beats. All positions depend on seconds, not frame count.
    float swatterX = glm::mix(-0.38f, -0.12f, sceneProgress(6.0f, 9.0f));
    float insectX = glm::mix(1.15f, 0.35f, sceneProgress(3.0f, 6.0f));
    float attack = 0.0f;
    if (cutsceneTime >= 9.0f && cutsceneTime < 12.0f)
    {
        attack = std::sin(sceneProgress(9.0f, 10.5f) * 3.14159265f);
        swatterX += 0.16f * attack;
        if (cutsceneTime < 9.8f)
            insectX = glm::mix(0.35f, 0.12f, sceneProgress(9.0f, 9.8f));
        else
            insectX = glm::mix(0.12f, 0.65f, sceneProgress(9.8f, 12.0f));
    }
    else if (cutsceneTime >= 12.0f)
    {
        swatterX = glm::mix(-0.12f, -0.22f, sceneProgress(12.0f, 13.5f));
        insectX = glm::mix(0.65f, 0.80f, sceneProgress(12.0f, 16.0f));
    }

    float swatterY = -0.05f + 0.012f * std::sin(cutsceneTime * 5.0f);
    drawSwatterGuard(transformLocation, colorLocation, circleVertexStart,
        circleVertexCount, triangleVertexStart, swatterX, swatterY, attack);
    if (cutsceneTime >= 3.0f)
    {
        float insectY = 0.02f + 0.025f * std::sin(cutsceneTime * 7.0f);
        drawCutsceneInsect(transformLocation, colorLocation, circleVertexStart,
            circleVertexCount, insectX, insectY, 0.19f);
    }

    // Contact flash makes the block visible even without the dialogue.
    if (cutsceneTime >= 9.8f && cutsceneTime < 10.2f)
    {
        glUniform3f(colorLocation, 1.0f, 0.88f, 0.25f);
        drawRectangle(transformLocation, 0.12f, 0.02f, 0.16f, 0.024f, 45.0f);
        drawRectangle(transformLocation, 0.12f, 0.02f, 0.16f, 0.024f, -45.0f);
    }
    if (cutsceneTime >= 12.0f)
    {
        for (int insect = 0; insect < 2; insect++)
        {
            float x = glm::mix(1.12f, 0.50f + insect * 0.25f, sceneProgress(12.0f + insect, 16.0f));
            float y = 0.24f + insect * 0.15f + 0.015f * std::sin(cutsceneTime * 8.0f + insect);
            drawCutsceneInsect(transformLocation, colorLocation, circleVertexStart,
                circleVertexCount, x, y, 0.11f);
        }
    }

    std::string heading;
    std::string line1;
    std::string line2;
    if (cutsceneTime < 3.0f)
    {
        heading = "THE FAMILY DINNER";
        line1 = "FRESH FOOD FOR THE FAMILY";
        line2 = "KEEP IT CLEAN UNTIL DINNER";
    }
    else if (cutsceneTime < 6.0f)
    {
        heading = "UNINVITED GUESTS";
        line1 = "INSECT   THAT FOOD WILL BE OURS";
        line2 = "SWATTER   STAY AWAY FROM THEIR DINNER";
    }
    else if (cutsceneTime < 9.0f)
    {
        heading = "THE KITCHEN GUARD";
        line1 = "SWATTER   THIS MEAL IS FOR THE FAMILY";
        line2 = "YOU WILL NOT REACH THE PLATE";
    }
    else if (cutsceneTime < 12.0f)
    {
        heading = "NOT ONE CRUMB";
        line1 = "INSECT   BRING THE WHOLE SWARM";
        line2 = "SWATTER   WE WILL PROTECT THIS KITCHEN";
    }
    else
    {
        heading = "DEFEND EVERY LANE";
        line1 = "SWATTER   GUARD EVERY LANE";
        line2 = "KEEP THE INSECTS AWAY FROM OUR FOOD";
    }

    // Text is drawn last, as taught in the renderer guide.
    glUniform3f(colorLocation, 1.0f, 0.88f, 0.38f);
    drawSceneText(transformLocation, "KITCHEN DEFENSE", 0.86f, 0.012f);
    glUniform3f(colorLocation, 0.84f, 0.94f, 0.96f);
    drawSceneText(transformLocation, heading, 0.70f, 0.0065f);
    glUniform3f(colorLocation, 0.20f, 0.28f, 0.31f);
    drawRectangle(transformLocation, 0.0f, -0.74f, 1.88f, 0.28f);
    glUniform3f(colorLocation, 1.0f, 0.91f, 0.55f);
    drawSceneText(transformLocation, line1, -0.69f, 0.0065f);
    glUniform3f(colorLocation, 0.95f, 0.97f, 0.98f);
    drawSceneText(transformLocation, line2, -0.79f, 0.0060f);

    float progress = cutsceneTime / CUTSCENE_DURATION;
    glUniform3f(colorLocation, 0.27f, 0.36f, 0.39f);
    drawRectangle(transformLocation, 0.0f, -0.94f, 1.70f, 0.025f);
    glUniform3f(colorLocation, 0.96f, 0.78f, 0.28f);
    drawRectangle(transformLocation, -0.85f + progress * 0.85f, -0.94f, progress * 1.70f, 0.025f);
}
