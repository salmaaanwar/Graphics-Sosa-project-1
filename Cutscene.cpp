#include "Cutscene.h"
#include "Renderer.h"
#include "TextRenderer.h"
#include "UI.h"
#include <cmath>
#include <string>
#include <algorithm>

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

static void drawKitchen(GLint transformLocation, GLint colorLocation,
    int circleVertexStart, int circleVertexCount)
{
    // The shared kitchen image already supplies the wall and counter.
    // Use the same separate food and plate PNGs as the gameplay home base.
    drawFoodPlate(transformLocation, colorLocation, FOOD_BREAD, -0.83f, -0.07f, 0.19f);
    drawFoodPlate(transformLocation, colorLocation, FOOD_APPLE, -0.65f, -0.07f, 0.19f);
    drawFoodPlate(transformLocation, colorLocation, FOOD_CHEESE, -0.47f, -0.07f, 0.19f);

    // Small rising steam puffs keep the establishing shot animated as well.
    glUniform3f(colorLocation, 0.98f, 0.98f, 0.87f);
    for (int puff = 0; puff < 3; puff++)
    {
        float rise = std::fmod(cutsceneTime * 0.12f + puff * 0.06f, 0.18f);
        float sway = 0.012f * std::sin(cutsceneTime * 3.0f + puff);
        drawIntroImage(transformLocation,colorLocation,"intro-steam",
                       -0.75f+puff*0.10f+sway,0.10f+rise,0.04f,0.04f);
    }
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
    int swatterFrame = attack > 0 ? 4 + std::min(3,int(attack*3.99f)) : int(cutsceneTime*8)%4;
    drawIntroImage(transformLocation,colorLocation,"intro-swatter-"+std::to_string(swatterFrame),
                   swatterX,swatterY,2,2);
    if (cutsceneTime >= 3.0f)
    {
        float insectY = 0.02f + 0.025f * std::sin(cutsceneTime * 7.0f);
        drawIntroImage(transformLocation,colorLocation,"intro-fly-"+std::to_string(int(cutsceneTime*20)%4),
                       insectX,insectY,0.38f,0.38f);
    }

    // Contact flash makes the block visible even without the dialogue.
    if (cutsceneTime >= 9.8f && cutsceneTime < 10.2f)
    {
        glUniform3f(colorLocation, 1.0f, 0.88f, 0.25f);
        drawIntroImage(transformLocation,colorLocation,"intro-flash",0.12f,0.02f,0.20f,0.20f);
    }
    if (cutsceneTime >= 12.0f)
    {
        for (int insect = 0; insect < 2; insect++)
        {
            float x = glm::mix(1.12f, 0.50f + insect * 0.25f, sceneProgress(12.0f + insect, 16.0f));
            float y = 0.24f + insect * 0.15f + 0.015f * std::sin(cutsceneTime * 8.0f + insect);
            drawIntroImage(transformLocation,colorLocation,"intro-fly-"+std::to_string(int(cutsceneTime*20)%4),
                           x,y,0.22f,0.22f);
        }
    }

    int beat = cutsceneTime < 3 ? 0 : cutsceneTime < 6 ? 1 : cutsceneTime < 9 ? 2 : cutsceneTime < 12 ? 3 : 4;
    // A wooden sign on the backsplash and a cream recipe card on the counter.
    drawIntroImage(transformLocation,colorLocation,"intro-heading-"+std::to_string(beat),
                   -0.10f,0.82f,1.50f,0.28125f);
    drawIntroImage(transformLocation,colorLocation,"intro-dialogue-"+std::to_string(beat),
                   0,-0.53f,1.88f,0.3525f);
    float progress=cutsceneTime/CUTSCENE_DURATION;
    drawIntroImage(transformLocation,colorLocation,"intro-progress",0,-0.74f,1.70f,0.012f);
    if(progress>0) drawIntroImage(transformLocation,colorLocation,"intro-progress",
                                 -0.85f+progress*0.85f,-0.74f,progress*1.70f,0.025f);
}
