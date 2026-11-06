// Offline exporter: the same solid-color shapes used by Player.cpp and UI.cpp.
// The running game loads separate PNGs and never calls these modeling functions.
#include "Renderer.h"
#include "UI.h"
#include "Player.h"
#include "Enemy.h"
#include "TextRenderer.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/glfw-3.5.1/deps/stb_image_write.h"

extern bool artworkBackdropOnly;
void drawButton(GLint, GLint, float, float, float, float, const std::string&,
                float, bool, float, float, float);
static GLint transformLocation, colorLocation;
static constexpr int circleStart = 6, circleCount = 52, triangleStart = 58;

static void color(float r, float g, float b) { glUniform3f(colorLocation, r, g, b); }
static void rect(float x, float y, float w, float h, float angle = 0)
{
    drawRectangle(transformLocation, x, y, w, h, angle);
}
static void circle(float x, float y, float radius)
{
    drawCircle(transformLocation, circleStart, circleCount, x, y, radius);
}
static void triangle(float x, float y, float w, float h, float angle = 0)
{
    drawTriangle(transformLocation, triangleStart, x, y, w, h, angle);
}

static float cutsceneTime = 0;
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

static void bakePlayerColor(GLint location, float r, float g, float b, int, int)
{
    glUniform3f(location,r,g,b);
}

// detailed melee fly swatter player model
void bakedrawMeleePlayer(
    const Player& p,
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart
) {
    float scaleX = 0.085f;
    float scaleY = 0.085f;
    if (p.pose == POSE_DUCKING) {
        scaleY = 0.055f; // crouch down when ducking
    }

    float step = 0.0f;
    if (!p.blockedByEnemy) {
        step = 0.012f * sinf(p.animTime * 8.0f);
    }
    float strike = p.attackAnimation > 0.0f
        ? sinf((1.0f - p.attackAnimation / PLAYER_ATTACK_ANIMATION_TIME) * 3.14159265f) : 0.0f;
    float armSwing = -65.0f * strike;
    float headX = p.x + 0.040f * strike;
    float headY = p.y - 0.020f * strike;

    // walking feet
    bakePlayerColor(colorLocation, 0.18f, 0.20f, 0.24f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x - 0.22f * scaleX + step, p.y - 0.98f * scaleY, 0.30f * scaleX, 0.18f * scaleY);
    drawRectangle(transformLocation, p.x + 0.22f * scaleX - step, p.y - 0.98f * scaleY, 0.30f * scaleX, 0.18f * scaleY);

    // handle grip
    bakePlayerColor(colorLocation, 0.25f, 0.25f, 0.30f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x, p.y - 0.50f * scaleY, 0.18f * scaleX, 0.75f * scaleY);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x, p.y - 0.85f * scaleY, 0.10f * scaleX);

    // neck connector
    bakePlayerColor(colorLocation, 0.85f, 0.25f, 0.30f, p.hp, p.maxHp);
    drawTriangle(transformLocation, triangleVertexStart, p.x, p.y - 0.08f * scaleY, 0.52f * scaleX, 0.35f * scaleY, 180.0f);

    // swatter head outer frame
    bakePlayerColor(colorLocation, 0.95f, 0.28f, 0.32f, p.hp, p.maxHp);
    drawRectangle(transformLocation, headX, headY + 0.38f * scaleY, 0.96f * scaleX, 0.86f * scaleY);

    // inner mesh background
    bakePlayerColor(colorLocation, 0.55f, 0.12f, 0.16f, p.hp, p.maxHp);
    drawRectangle(transformLocation, headX, headY + 0.38f * scaleY, 0.76f * scaleX, 0.66f * scaleY);

    // mesh grid lines
    bakePlayerColor(colorLocation, 1.0f, 0.55f, 0.58f, p.hp, p.maxHp);
    drawRectangle(transformLocation, headX - 0.20f * scaleX, headY + 0.38f * scaleY, 0.06f * scaleX, 0.66f * scaleY);
    drawRectangle(transformLocation, headX,                  headY + 0.38f * scaleY, 0.06f * scaleX, 0.66f * scaleY);
    drawRectangle(transformLocation, headX + 0.20f * scaleX, headY + 0.38f * scaleY, 0.06f * scaleX, 0.66f * scaleY);
    drawRectangle(transformLocation, headX, headY + 0.18f * scaleY, 0.76f * scaleX, 0.06f * scaleY);
    drawRectangle(transformLocation, headX, headY + 0.38f * scaleY, 0.76f * scaleX, 0.06f * scaleY);
    drawRectangle(transformLocation, headX, headY + 0.58f * scaleY, 0.76f * scaleX, 0.06f * scaleY);

    // eyes looking right
    bakePlayerColor(colorLocation, 0.98f, 0.98f, 0.95f, p.hp, p.maxHp);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, headX - 0.14f * scaleX, headY + 0.44f * scaleY, 0.13f * scaleX);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, headX + 0.18f * scaleX, headY + 0.44f * scaleY, 0.13f * scaleX);

    bakePlayerColor(colorLocation, 0.10f, 0.14f, 0.18f, p.hp, p.maxHp);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, headX - 0.10f * scaleX, headY + 0.44f * scaleY, 0.06f * scaleX);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, headX + 0.22f * scaleX, headY + 0.44f * scaleY, 0.06f * scaleX);

    // swinging weapon arm in front
    bakePlayerColor(colorLocation, 0.92f, 0.30f, 0.32f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x + 0.48f * scaleX, p.y - 0.18f * scaleY, 0.55f * scaleX, 0.14f * scaleY, armSwing);
}

// detailed long range spray bottle player model
void bakedrawRangedPlayer(
    const Player& p,
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart
) {
    float scaleX = 0.085f;
    float scaleY = 0.085f;
    if (p.pose == POSE_DUCKING) {
        scaleY = 0.055f; // crouch down when ducking
    }

    float step = 0.0f;
    if (!p.blockedByEnemy) {
        step = 0.012f * sinf(p.animTime * 8.0f);
    }
    float sprayPulse = p.attackAnimation > 0.0f
        ? sinf((1.0f - p.attackAnimation / PLAYER_ATTACK_ANIMATION_TIME) * 3.14159265f) : 0.0f;
    float triggerPull = -22.0f * sprayPulse;

    // walking feet
    bakePlayerColor(colorLocation, 0.14f, 0.35f, 0.55f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x - 0.22f * scaleX + step, p.y - 0.90f * scaleY, 0.30f * scaleX, 0.16f * scaleY);
    drawRectangle(transformLocation, p.x + 0.18f * scaleX - step, p.y - 0.90f * scaleY, 0.30f * scaleX, 0.16f * scaleY);

    // bottle body and shoulder taper
    bakePlayerColor(colorLocation, 0.18f, 0.68f, 0.95f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x - 0.05f * scaleX, p.y - 0.38f * scaleY, 0.78f * scaleX, 0.88f * scaleY);
    drawTriangle(transformLocation, triangleVertexStart, p.x - 0.05f * scaleX, p.y + 0.22f * scaleY, 0.78f * scaleX, 0.38f * scaleY);

    // bottle label and stripe
    bakePlayerColor(colorLocation, 0.95f, 0.98f, 1.0f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x - 0.05f * scaleX, p.y - 0.42f * scaleY, 0.56f * scaleX, 0.38f * scaleY);

    bakePlayerColor(colorLocation, 0.10f, 0.45f, 0.80f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x - 0.05f * scaleX, p.y - 0.42f * scaleY, 0.42f * scaleX, 0.12f * scaleY);

    // eyes on bottle
    bakePlayerColor(colorLocation, 1.0f, 1.0f, 1.0f, p.hp, p.maxHp);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x - 0.16f * scaleX, p.y - 0.06f * scaleY, 0.11f * scaleX);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x + 0.10f * scaleX, p.y - 0.06f * scaleY, 0.11f * scaleX);

    bakePlayerColor(colorLocation, 0.10f, 0.15f, 0.22f, p.hp, p.maxHp);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x - 0.12f * scaleX, p.y - 0.06f * scaleY, 0.05f * scaleX);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x + 0.14f * scaleX, p.y - 0.06f * scaleY, 0.05f * scaleX);

    // yellow neck collar
    bakePlayerColor(colorLocation, 0.95f, 0.85f, 0.20f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x - 0.05f * scaleX, p.y + 0.42f * scaleY, 0.32f * scaleX, 0.16f * scaleY);

    // spray head and nozzle
    bakePlayerColor(colorLocation, 0.95f, 0.30f, 0.30f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x + 0.04f * scaleX, p.y + 0.62f * scaleY, 0.64f * scaleX, 0.24f * scaleY);

    // animated trigger lever
    bakePlayerColor(colorLocation, 0.88f, 0.18f, 0.18f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x + 0.24f * scaleX, p.y + 0.38f * scaleY, 0.10f * scaleX, 0.32f * scaleY, 22.0f + triggerPull);

    // Mist appears only while the trigger is firing; moving shots are separate.
    if (p.attackAnimation > 0.0f) {
        bakePlayerColor(colorLocation, 0.55f, 0.92f, 1.0f, p.hp, p.maxHp);
        drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x + (0.50f + sprayPulse * 0.12f) * scaleX, p.y + 0.66f * scaleY, 0.07f * scaleX);
        drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x + (0.64f + sprayPulse * 0.12f) * scaleX, p.y + 0.56f * scaleY, 0.05f * scaleX);
    }
}

// detailed stationary defense sticky sheet player model
void bakedrawDefensePlayer(
    const Player& p,
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount
) {
    float scale = 0.085f;

    // sturdy bottom stand
    bakePlayerColor(colorLocation, 0.45f, 0.28f, 0.12f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x, p.y - 0.78f * scale, 1.08f * scale, 0.18f * scale);

    // outer cardboard frame
    bakePlayerColor(colorLocation, 0.65f, 0.42f, 0.18f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x, p.y - 0.05f * scale, 0.88f * scale, 1.35f * scale);

    // top hanging roller
    bakePlayerColor(colorLocation, 0.82f, 0.58f, 0.28f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x, p.y + 0.68f * scale, 0.98f * scale, 0.20f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x - 0.49f * scale, p.y + 0.68f * scale, 0.10f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x + 0.49f * scale, p.y + 0.68f * scale, 0.10f * scale);

    // yellow sticky glue surface
    bakePlayerColor(colorLocation, 0.98f, 0.88f, 0.22f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x, p.y - 0.08f * scale, 0.68f * scale, 1.08f * scale);

    // glue drops
    bakePlayerColor(colorLocation, 0.92f, 0.65f, 0.10f, p.hp, p.maxHp);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x - 0.15f * scale, p.y + 0.20f * scale, 0.12f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x + 0.16f * scale, p.y - 0.05f * scale, 0.14f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x - 0.10f * scale, p.y - 0.35f * scale, 0.11f * scale);

    // green defense shield cross
    bakePlayerColor(colorLocation, 0.35f, 0.80f, 0.30f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x, p.y - 0.08f * scale, 0.42f * scale, 0.12f * scale);
    drawRectangle(transformLocation, p.x, p.y - 0.08f * scale, 0.12f * scale, 0.42f * scale);
}

// ant with a slingshot
void bakeDrawAnt(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    float x,
    float y,
    float attackAnimation)
{
    // legs
    glUniform3f(colorLocation, 0.20f, 0.10f, 0.05f);

    for (int leg = 0; leg < 3; leg++)
    {
        float legX = x - 0.02f + leg * 0.025f;

        drawRectangle(transformLocation,
                      legX, y - 0.03f, 0.008f, 0.05f, 35.0f);

        drawRectangle(transformLocation,
                      legX, y - 0.03f, 0.008f, 0.05f, -35.0f);
    }

    // abdomen, middle body, and head
    glUniform3f(colorLocation, 0.45f, 0.18f, 0.08f);

    drawCircle(transformLocation,
               circleVertexStart, circleVertexCount,
               x + 0.035f, y, 0.028f);

    drawCircle(transformLocation,
               circleVertexStart, circleVertexCount,
               x, y, 0.018f);

    drawCircle(transformLocation,
               circleVertexStart, circleVertexCount,
               x - 0.035f, y, 0.023f);

    // antennae
    glUniform3f(colorLocation, 0.20f, 0.10f, 0.05f);

    drawRectangle(transformLocation,
                  x - 0.05f, y + 0.028f, 0.005f, 0.035f, 30.0f);

    drawRectangle(transformLocation,
                  x - 0.025f, y + 0.028f, 0.005f, 0.035f, -30.0f);

    // eye
    glUniform3f(colorLocation, 0.95f, 0.85f, 0.65f);

    drawCircle(transformLocation,
               circleVertexStart, circleVertexCount,
               x - 0.045f, y + 0.005f, 0.006f);

    // slingshot recoil
    float slingX = x - 0.075f + attackAnimation * 0.08f;

    glUniform3f(colorLocation, 0.35f, 0.16f, 0.05f);

    drawRectangle(transformLocation,
                  slingX, y - 0.07f, 0.012f, 0.04f);

    drawRectangle(transformLocation,
                  slingX - 0.009f, y - 0.045f,
                  0.01f, 0.035f, 30.0f);

    drawRectangle(transformLocation,
                  slingX + 0.009f, y - 0.045f,
                  0.01f, 0.035f, -30.0f);

    // rubber band
    glUniform3f(colorLocation, 0.10f, 0.10f, 0.15f);

    drawRectangle(transformLocation,
                  slingX, y - 0.03f, 0.035f, 0.005f);
}

// cockroach with a claymore
void bakeDrawCockroach(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float x,
    float y,
    float attackAnimation)
{
    // six legs
    glUniform3f(colorLocation, 0.18f, 0.08f, 0.03f);

    for (int leg = 0; leg < 3; leg++)
    {
        float legY = y + 0.025f - leg * 0.025f;

        drawRectangle(transformLocation,
                      x - 0.045f, legY, 0.045f, 0.008f, 25.0f);

        drawRectangle(transformLocation,
                      x + 0.045f, legY, 0.045f, 0.008f, -25.0f);
    }

    // body
    glUniform3f(colorLocation, 0.30f, 0.12f, 0.04f);

    drawCircle(transformLocation,
               circleVertexStart, circleVertexCount,
               x, y - 0.015f, 0.04f);

    drawRectangle(transformLocation,
                  x, y + 0.005f, 0.065f, 0.07f);

    // wing covers
    glUniform3f(colorLocation, 0.55f, 0.28f, 0.10f);

    drawRectangle(transformLocation,
                  x - 0.016f, y, 0.026f, 0.07f, -8.0f);

    drawRectangle(transformLocation,
                  x + 0.016f, y, 0.026f, 0.07f, 8.0f);

    // head and antennae
    glUniform3f(colorLocation, 0.18f, 0.08f, 0.03f);

    drawCircle(transformLocation,
               circleVertexStart, circleVertexCount,
               x - 0.025f, y + 0.04f, 0.023f);

    drawRectangle(transformLocation,
                  x - 0.045f, y + 0.075f, 0.005f, 0.06f, 30.0f);

    drawRectangle(transformLocation,
                  x - 0.015f, y + 0.075f, 0.005f, 0.06f, -20.0f);

    // eye
    glUniform3f(colorLocation, 0.95f, 0.15f, 0.10f);

    drawCircle(transformLocation,
               circleVertexStart, circleVertexCount,
               x - 0.035f, y + 0.045f, 0.006f);

    // sword thrust toward the player
    float swordX = x - 0.075f - attackAnimation * 0.15f;

    // blade and pointed tip
    glUniform3f(colorLocation, 0.70f, 0.80f, 0.90f);

    drawRectangle(transformLocation,
                  swordX, y + 0.015f, 0.018f, 0.10f);

    drawTriangle(transformLocation, triangleVertexStart,
                 swordX, y + 0.075f, 0.018f, 0.02f);

    // guard and handle
    glUniform3f(colorLocation, 0.25f, 0.18f, 0.10f);

    drawRectangle(transformLocation,
                  swordX, y - 0.04f, 0.05f, 0.01f);

    drawRectangle(transformLocation,
                  swordX, y - 0.06f, 0.012f, 0.03f);
}

static void pot()
{
    color(0.22f, 0.30f, 0.32f); rect(0, -0.12f, 1.16f, 1.00f);
    color(0.52f, 0.58f, 0.61f); rect(0, -0.10f, 1.06f, 0.90f);
    // Three bars per handle keep the centers genuinely transparent.
    for (float side : {-1.0f, 1.0f}) {
        color(0.22f, 0.30f, 0.32f);
        rect(side * 0.68f, 0.13f, 0.26f, 0.07f);
        rect(side * 0.68f, -0.10f, 0.26f, 0.07f);
        rect(side * 0.79f, 0.015f, 0.07f, 0.26f);
    }
    color(0.60f, 0.65f, 0.65f); triangle(0, 0.51f, 1.14f, 0.22f);
    color(0.93f, 0.85f, 0.69f); rect(0, 0.39f, 1.24f, 0.07f);
    color(0.22f, 0.30f, 0.32f); rect(0, 0.67f, 0.17f, 0.11f);
}

static void herb()
{
    color(0.18f, 0.42f, 0.14f);
    rect(0, 0.20f, 0.04f, 0.80f);
    rect(-0.16f, 0.12f, 0.035f, 0.64f, 28);
    rect(0.16f, 0.20f, 0.035f, 0.70f, -28);
    color(0.34f, 0.61f, 0.23f);
    triangle(-0.30f, 0.30f, 0.34f, 0.38f, 65);
    triangle(0.31f, 0.44f, 0.34f, 0.38f, -65);
    triangle(-0.16f, 0.62f, 0.32f, 0.35f, 45);
    color(0.18f, 0.42f, 0.14f);
    triangle(0.18f, 0.12f, 0.31f, 0.33f, -65);
    triangle(-0.22f, 0.04f, 0.29f, 0.30f, 65);
    color(0.77f, 0.29f, 0.22f); rect(0, -0.43f, 0.56f, 0.47f);
    color(0.88f, 0.43f, 0.29f); rect(0, -0.20f, 0.70f, 0.13f);
    color(0.57f, 0.27f, 0.17f); rect(0, -0.69f, 0.64f, 0.07f);
}

static void jar()
{
    color(0.55f, 0.32f, 0.19f); rect(0, -0.14f, 0.94f, 1.08f);
    color(0.93f, 0.85f, 0.69f); rect(0, -0.14f, 0.84f, 0.98f);
    color(0.77f, 0.56f, 0.34f); rect(0, 0.46f, 1.03f, 0.18f);
    color(0.55f, 0.32f, 0.19f); rect(0, 0.61f, 0.18f, 0.12f);
    color(0.77f, 0.29f, 0.22f); rect(0, 0.24f, 0.84f, 0.07f);
    rect(0, -0.48f, 0.84f, 0.07f);
    color(0.88f, 0.62f, 0.25f); circle(0, -0.13f, 0.22f);
    color(0.43f, 0.25f, 0.14f);
    circle(-0.08f, -0.05f, 0.035f); circle(0.09f, -0.10f, 0.035f);
    circle(-0.03f, -0.23f, 0.035f);
}

static void toaster()
{
    color(0.43f, 0.25f, 0.14f); rect(0, -0.13f, 1.32f, 0.98f);
    color(0.77f, 0.29f, 0.22f); rect(0, -0.13f, 1.22f, 0.88f);
    color(0.88f, 0.43f, 0.29f); rect(0, 0.27f, 1.22f, 0.17f);
    color(0.18f, 0.20f, 0.24f);
    rect(-0.29f, 0.29f, 0.43f, 0.055f); rect(0.29f, 0.29f, 0.43f, 0.055f);
    rect(-0.44f, -0.67f, 0.20f, 0.10f); rect(0.44f, -0.67f, 0.20f, 0.10f);
    rect(0.65f, -0.02f, 0.06f, 0.30f);
    color(0.93f, 0.85f, 0.69f); rect(0.74f, 0.07f, 0.20f, 0.09f);
    circle(0.33f, -0.28f, 0.105f);
    color(0.43f, 0.25f, 0.14f); rect(0.33f, -0.28f, 0.025f, 0.13f, -30);
}

static void mug()
{
    color(0.93f, 0.85f, 0.69f);
    rect(-0.12f, -0.16f, 0.92f, 0.98f);
    rect(0.53f, 0.20f, 0.39f, 0.10f);
    rect(0.53f, -0.37f, 0.39f, 0.10f);
    rect(0.70f, -0.085f, 0.10f, 0.67f);
    color(0.43f, 0.65f, 0.78f);
    rect(-0.12f, 0.36f, 0.97f, 0.10f);
    rect(-0.12f, -0.60f, 0.92f, 0.09f);
    color(0.43f, 0.25f, 0.14f); rect(-0.12f, 0.38f, 0.76f, 0.045f);
}

static void plate()
{
    color(0.65f, 0.57f, 0.43f); circle(0, 0, 0.90f);
    color(1.0f, 0.96f, 0.83f); circle(0, 0, 0.82f);
}

static void bread()
{
    color(0.75f, 0.47f, 0.22f); circle(0, 0, 0.57f);
    color(0.88f, 0.62f, 0.25f); circle(0, 0, 0.51f);
    color(1.0f, 0.82f, 0.52f);
    rect(-0.19f, 0.03f, 0.07f, 0.49f, -20);
    rect(0.19f, 0.03f, 0.07f, 0.49f, -20);
}

static void apple()
{
    color(0.75f, 0.12f, 0.10f); circle(0, -0.03f, 0.50f);
    color(0.43f, 0.25f, 0.14f); rect(0.02f, 0.51f, 0.08f, 0.27f, -15);
    color(0.18f, 0.42f, 0.14f); triangle(0.22f, 0.53f, 0.30f, 0.20f, -60);
    color(1.0f, 0.82f, 0.52f); circle(-0.22f, 0.18f, 0.055f);
}

static void cheese()
{
    color(0.78f, 0.47f, 0.10f); triangle(0, 0, 1.17f, 1.13f);
    color(1.0f, 0.82f, 0.28f); triangle(0, 0, 1.08f, 1.04f);
    color(0.78f, 0.47f, 0.10f);
    circle(-0.22f, -0.25f, 0.07f); circle(0.18f, -0.28f, 0.07f);
    circle(0, 0.05f, 0.07f);
}

static GLuint shader(GLenum type, const char* source)
{
    GLuint result = glCreateShader(type);
    glShaderSource(result, 1, &source, nullptr);
    glCompileShader(result);
    GLint compiled;
    glGetShaderiv(result, GL_COMPILE_STATUS, &compiled);
    if (!compiled) { std::cerr << "Exporter shader failed\n"; std::exit(1); }
    return result;
}

int main(int argc, char** argv)
{
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(64, 64, "Kitchen image export", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress)) return 1;

    GLuint vs = shader(GL_VERTEX_SHADER, "#version 330 core\nlayout(location=0) in vec3 p; uniform mat4 u_Transform; uniform mat4 u_Projection; void main(){gl_Position=u_Projection*u_Transform*vec4(p,1);}");
    GLuint fs = shader(GL_FRAGMENT_SHADER, "#version 330 core\nout vec4 result; uniform vec3 uColor; void main(){result=vec4(uColor,1);}");
    GLuint program = glCreateProgram();
    glAttachShader(program, vs); glAttachShader(program, fs); glLinkProgram(program);
    GLint linked; glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) return 1;
    glUseProgram(program);
    transformLocation = glGetUniformLocation(program, "u_Transform");
    colorLocation = glGetUniformLocation(program, "uColor");
    GLint projectionLocation = glGetUniformLocation(program, "u_Projection");
    glm::mat4 identity(1);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(identity));

    std::vector<float> vertices = {-.5f,-.5f,0, .5f,-.5f,0, .5f,.5f,0,
                                  -.5f,-.5f,0, .5f,.5f,0, -.5f,.5f,0, 0,0,0};
    for (int i = 0; i <= 50; ++i) {
        float a = 6.2831853f * i / 50;
        vertices.insert(vertices.end(), {std::cos(a), std::sin(a), 0});
    }
    vertices.insert(vertices.end(), {-.5f,-.5f,0, .5f,-.5f,0, 0,.5f,0});
    GLuint vao, vbo, framebuffer, texture;
    glGenVertexArrays(1, &vao); glBindVertexArray(vao);
    glGenBuffers(1, &vbo); glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glGenTextures(1, &texture); glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glGenFramebuffers(1, &framebuffer); glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return 1;
    glViewport(0, 0, 512, 512);
    glClearColor(0, 0, 0, 0);
    stbi_flip_vertically_on_write(true);
    std::filesystem::path output = argc > 1 ? argv[1] : GAME_ASSET_DIR;
    std::filesystem::create_directories(output);
    struct Asset { const char* filename; void (*draw)(); };
    for (const Asset& asset : {Asset{"kitchen-pot.png", pot}, {"kitchen-herb.png", herb},
                              {"kitchen-jar.png", jar}, {"kitchen-toaster.png", toaster}, {"kitchen-mug.png", mug},
                              {"food-plate.png", plate}, {"food-bread.png", bread},
                              {"food-apple.png", apple}, {"food-cheese.png", cheese}}) {
        glClear(GL_COLOR_BUFFER_BIT);
        asset.draw();
        std::vector<unsigned char> pixels(512 * 512 * 4);
        glReadPixels(0, 0, 512, 512, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        if (glGetError() != GL_NO_ERROR || !stbi_write_png((output / asset.filename).string().c_str(),
                                                        512, 512, 4, pixels.data(), 512 * 4)) return 1;
        std::cout << "Exported " << asset.filename << '\n';
    }
    auto save = [&](const std::string& name, int width, int height, auto draw) {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glViewport(0, 0, width, height);
        glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -float(height)/width, float(height)/width);
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
        glClear(GL_COLOR_BUFFER_BIT);
        draw();
        std::vector<unsigned char> pixels(width * height * 4);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        if (glGetError() != GL_NO_ERROR || !stbi_write_png((output/name).string().c_str(), width, height, 4, pixels.data(), width*4)) std::exit(1);
    };
    auto centered = [&](const std::string& text, float y, float size) {
        drawText(transformLocation, text, -(6.0f*text.size()-1)*size/2, y+3.5f*size, size);
    };
    for (int frame=0; frame<8; ++frame) {
        save("intro-swatter-"+std::to_string(frame)+".png", 1024, 1024, [&] {
            cutsceneTime=frame*0.15f;
            drawSwatterGuard(transformLocation,colorLocation,circleStart,circleCount,triangleStart,0,0,frame>=4 ? (frame-3)/4.0f : 0);
        });
    }
    for (int frame=0; frame<4; ++frame) {
        save("intro-fly-"+std::to_string(frame)+".png",512,512,[&] {
            cutsceneTime=frame*0.05236f;
            drawCutsceneInsect(transformLocation,colorLocation,circleStart,circleCount,0,0,1);
        });
    }
    const char* headings[]={"THE FAMILY DINNER","UNINVITED GUESTS","THE KITCHEN GUARD","NOT ONE CRUMB","DEFEND EVERY LANE"};
    const char* first[]={"FRESH FOOD FOR THE FAMILY","INSECT   THAT FOOD WILL BE OURS","SWATTER   THIS MEAL IS FOR THE FAMILY","INSECT   BRING THE WHOLE SWARM","SWATTER   GUARD EVERY LANE"};
    const char* second[]={"KEEP IT CLEAN UNTIL DINNER","SWATTER   STAY AWAY FROM THEIR DINNER","YOU WILL NOT REACH THE PLATE","SWATTER   WE WILL PROTECT THIS KITCHEN","KEEP THE INSECTS AWAY FROM OUR FOOD"};
    for(int beat=0;beat<5;++beat) {
        save("intro-heading-"+std::to_string(beat)+".png",1024,192,[&] {
            color(.43f,.25f,.14f); rect(0,0,1.98f,.365f);
            color(.77f,.56f,.34f); rect(0,0,1.94f,.325f);
            color(1,.96f,.83f); centered("KITCHEN DEFENSE",.045f,.014f);
            color(.18f,.20f,.24f); centered(headings[beat],-.09f,.008f);
        });
        save("intro-dialogue-"+std::to_string(beat)+".png",1024,192,[&] {
            color(.43f,.25f,.14f); rect(0,0,1.98f,.365f);
            color(1,.96f,.83f); rect(0,0,1.94f,.325f);
            color(.43f,.25f,.14f); centered(first[beat],.065f,.0073f);
            color(.18f,.30f,.32f); centered(second[beat],-.065f,.0070f);
        });
    }
    for(int hover=0;hover<2;++hover) save("intro-skip-"+std::to_string(hover)+".png",512,256,[&] {
        color(hover?1.0f:.43f,hover?.82f:.25f,hover?.28f:.14f); rect(0,0,1.98f,.98f);
        color(.77f,.56f,.34f);rect(0,0,1.86f,.86f);
        color(1,.96f,.83f);centered("SKIP",0,.045f);
    });
    save("intro-steam.png",256,256,[&]{color(.98f,.98f,.87f);circle(0,0,.7f);});
    save("intro-flash.png",256,256,[&]{color(1,.88f,.25f);rect(0,0,1.6f,.24f,45);rect(0,0,1.6f,.24f,-45);});
    save("intro-progress.png",512,64,[&]{color(.77f,.56f,.34f);rect(0,0,2,.25f);});
    auto spriteView = [&](float half) {
        glm::mat4 view=glm::ortho(-half,half,-half,half);
        glUniformMatrix4fv(projectionLocation,1,GL_FALSE,glm::value_ptr(view));
    };
    for(int type=0;type<3;++type) for(int pose : {POSE_NORMAL,POSE_DUCKING}) {
        if(type==PLAYER_DEFENSE && pose!=POSE_NORMAL) continue;
        for(int frame=0;frame<(type==PLAYER_DEFENSE ? 1 : 8);++frame) {
            save("player-"+std::to_string(type)+"-"+std::to_string(pose)+"-"+std::to_string(frame)+".png",512,512,[&]{
                spriteView(.20f);
                Player p{};p.type=type;p.pose=pose;p.hp=p.maxHp=7;
                p.animTime=frame*3.14159265f/16.0f;
                p.blockedByEnemy=frame>=4;
                p.attackAnimation=frame>=4 ? PLAYER_ATTACK_ANIMATION_TIME*(1-(frame-4+.5f)/4) : 0;
                if(type==PLAYER_MELEE) bakedrawMeleePlayer(p,transformLocation,colorLocation,circleStart,circleCount,triangleStart);
                else if(type==PLAYER_RANGED) bakedrawRangedPlayer(p,transformLocation,colorLocation,circleStart,circleCount,triangleStart);
                else bakedrawDefensePlayer(p,transformLocation,colorLocation,circleStart,circleCount);
            });
        }
    }
    for(int type=0;type<3;++type) for(int frame=0;frame<5;++frame) {
        save("enemy-"+std::to_string(type)+"-"+std::to_string(frame)+".png",512,512,[&]{
            spriteView(.20f);
            float attack=frame==0 ? 0 : .20f*(frame-.5f)/4;
            if(type==ENEMY_EASY) {
                drawMenuInsect(transformLocation,colorLocation,circleStart,circleCount,triangleStart,attack*.06f,.05f,.11f);
                if(frame) {color(1,.25f,.05f);circle(attack*.06f-.08f,.045f,.02f);}
            } else if(type==ENEMY_MEDIUM) bakeDrawAnt(transformLocation,colorLocation,circleStart,circleCount,0,0,attack);
            else bakeDrawCockroach(transformLocation,colorLocation,circleStart,circleCount,triangleStart,0,0,attack);
        });
    }
    save("projectile-spray.png",256,256,[&]{
        spriteView(.05f);color(.12f,.55f,.75f);rect(-.018f,0,.045f,.008f);
        color(.55f,.94f,1);circle(0,0,.012f);color(.94f,1,1);circle(.003f,.004f,.004f);
    });
    for(int type=0;type<2;++type) save("projectile-enemy-"+std::to_string(type)+".png",256,256,[&]{
        spriteView(.05f);if(type==0)color(.30f,.05f,.40f);else color(.15f,.15f,.18f);circle(0,0,.012f);
    });
    save("font.png",1024,512,[&] {
        color(1,1,1);
        for(int character=0;character<128;++character) {
            float x=-1.0f+((character%16)*64+12)*2.0f/1024;
            float y=0.5f-((character/16)*64+4)*1.0f/512;
            drawText(transformLocation,std::string(1,char(character)),x,y,8.0f*2/1024);
        }
    });
    artworkBackdropOnly=true;
    save("menu-main.png",1024,1024,[&]{drawMainMenu(transformLocation,colorLocation,circleStart,circleCount,triangleStart,-99,-99);});
    save("menu-pause.png",1024,1024,[&]{drawPauseMenu(transformLocation,colorLocation,-99,-99);});
    for(int win=0;win<2;++win) save(win ? "menu-win.png" : "menu-lose.png",1024,1024,[&]{
        drawWinLoseMenu(transformLocation,colorLocation,circleStart,circleCount,triangleStart,win,-99,-99);
    });
    artworkBackdropOnly=false;
    struct Button {const char* label;const char* key;float r,g,b;};
    for(const Button& button : {Button{"START GAME","START_GAME",.18f,.62f,.30f},
            {"EXIT GAME","EXIT_GAME",.72f,.22f,.22f},{"CONTINUE","CONTINUE",.20f,.62f,.32f},
            {"RESTART","RESTART",.25f,.48f,.75f},{"MAIN MENU","MAIN_MENU",.68f,.26f,.24f},
            {"PAUSE","PAUSE",.28f,.35f,.48f}})
        for(int hover=0;hover<2;++hover) {
            bool compact=std::string(button.key)=="PAUSE";
            save(std::string("button-")+button.key+"-"+std::to_string(hover)+".png",compact ? 512 : 1024,256,[&]{
                drawButton(transformLocation,colorLocation,0,0,1.98f,compact ? .92f : .46f,
                           button.label,compact ? .042f : .023f,hover,button.r,button.g,button.b);
            });
        }
    save("ui-fill.png",64,64,[&]{color(1,1,1);rect(0,0,2,2);});
    save("hud-bar.png",1024,128,[&]{
        color(.16f,.18f,.22f);rect(0,0,2,.25f);
        color(.78f,.65f,.42f);rect(0,-.12f,2,.01f);
    });
    save("hud-coins.png",1024,420,[&]{
        color(.85f,.72f,.35f);rect(0,0,2,420.0f/512);
        color(.12f,.15f,.20f);rect(0,0,1.91f,420.0f/512-.09f);
    });
    for(int state=0;state<4;++state) save("hud-card-"+std::to_string(state)+".png",512,532,[&]{
        if(state==1) color(.32f,.32f,.32f);
        else if(state==2)color(1,.92f,.25f);
        else if(state==3)color(.25f,1,.40f);
        else color(.85f,.72f,.45f);
        rect(0,0,2,532.0f/256);
        if(state==1) color(.22f,.22f,.22f);
        else if(state>=2)color(.28f,.36f,.46f);
        else color(.20f,.26f,.34f);
        rect(0,0,1.84f,532.0f/256-.16f);
    });
    for(int gray=0;gray<2;++gray) {
        save("icon-coin-"+std::to_string(gray)+".png",512,512,[&]{drawCollectibleIcon(transformLocation,colorLocation,circleStart,circleCount,0,0,.8f,gray);});
        save("icon-swatter-"+std::to_string(gray)+".png",512,512,[&]{drawSwatterIcon(transformLocation,colorLocation,circleStart,circleCount,triangleStart,0,0,.65f,gray);});
        save("icon-spray-"+std::to_string(gray)+".png",512,512,[&]{drawSprayBottleIcon(transformLocation,colorLocation,circleStart,circleCount,triangleStart,0,0,.65f,gray);});
        save("icon-sticky-"+std::to_string(gray)+".png",512,512,[&]{drawStickySheetIcon(transformLocation,colorLocation,circleStart,circleCount,0,0,.65f,gray);});
    }
    save("icon-insect.png",512,512,[&]{drawMenuInsect(transformLocation,colorLocation,circleStart,circleCount,triangleStart,0,0,.65f);});
    for(int wave=1;wave<=3;++wave) save("wave-"+std::to_string(wave)+".png",1024,192,[&]{
        color(.85f,.72f,.45f);rect(0,0,2,.375f);
        color(.16f,.20f,.28f);rect(0,0,1.93f,.30f);
        color(1,.92f,.30f);centered("WAVE "+std::to_string(wave)+" STARTING",0,.018f);
    });
    glDeleteFramebuffers(1, &framebuffer); glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &vbo); glDeleteVertexArrays(1, &vao);
    glDeleteShader(vs); glDeleteShader(fs); glDeleteProgram(program);
    glfwDestroyWindow(window); glfwTerminate();
}
