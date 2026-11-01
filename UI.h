#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include "Game.h"

// player card costs
const int MELEE_COST   = 10;
const int RANGED_COST  = 40;
const int DEFENSE_COST = 60;

// shared HUD state functions
int getCollectibles();
void setCollectibles(int amount);
int getSelectedCard();
void resetGamePlaceholders();

// composite icon drawing functions
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

void drawMenuInsect(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float x,
    float y,
    float scale
);

// unit health bar drawing
void drawHealthBar(
    GLint transformLocation,
    GLint colorLocation,
    float x,
    float y,
    int currentHP,
    int maxHP
);

// top HUD drawing
void drawHUD(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float mouseX,
    float mouseY
);

// menus drawing
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

// main UI draw and input functions
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
