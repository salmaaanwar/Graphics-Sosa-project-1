#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include "Game.h"
#include "Deployment.h"

void drawAudioControls(GLint transformLocation, GLint colorLocation);
bool handleAudioControlsMouse(float x, float y, int action);
void updateAudioControlsMouse(float x, float y, bool leftDown);
bool handleAudioControlsKey(int key, int action);

// Geometry arguments are shared with the offline artwork exporter.
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

void drawHealthBar(
    GLint transformLocation,
    GLint colorLocation,
    float x,
    float y,
    int currentHP,
    int maxHP
);

void drawHUD(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float mouseX,
    float mouseY
);

void drawMainMenu(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float mouseX,
    float mouseY
);

void drawCenteredText(GLint transformLocation, const std::string& text,
                      float centerX, float centerY, float cellSize);

void drawCutsceneSkipButton(GLint transformLocation, GLint colorLocation, float mouseX, float mouseY);

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
