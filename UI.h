#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include "Game.h"

// Player card costs (Melee = 10, Long Range = 40, Defense = 60)
const int MELEE_COST   = 10;
const int RANGED_COST  = 40;
const int DEFENSE_COST = 60;

// Game state functions come from Game.h; HUD values remain in this module.
int getCollectibles();
void setCollectibles(int amount);
int getSelectedCard();
void resetGamePlaceholders();

// Composite icon drawing functions (built from multiple primitives)
void drawCollectibleIcon(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    float x,
    float y,
    float scale,
    bool grayscale
);

void drawSwatterIcon(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float x,
    float y,
    float scale,
    bool grayscale
);

void drawSprayBottleIcon(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float x,
    float y,
    float scale,
    bool grayscale
);

void drawStickySheetIcon(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    float x,
    float y,
    float scale,
    bool grayscale
);

// Reusable unit health display (draws health bar + numerical HP above/below a unit)
void drawHealthBar(
    GLint transformLocation,
    GLint colorLocation,
    float x,
    float y,
    int currentHP,
    int maxHP
);

// Top HUD drawing (collectibles counter + 3 lock/unlock player cards)
void drawHUD(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float mouseX,
    float mouseY
);

// Placeholder battlefield preview to test HUD and health bars during STATE_PLAYING
void drawPlayingPlaceholderScene(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart
);

// Menus drawing
void drawMainMenu(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float mouseX,
    float mouseY
);

void drawPauseMenu(
    GLint transformLocation,
    GLint colorLocation,
    float mouseX,
    float mouseY
);

void drawWinLoseMenu(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    bool isWin,
    float mouseX,
    float mouseY
);

// Master UI draw & input functions
void drawUI(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float mouseX,
    float mouseY
);

void handleUIMouseClick(
    GLFWwindow* window,
    float mouseX,
    float mouseY
);

void handleUIKeyPress(
    GLFWwindow* window,
    int key,
    int action
);
