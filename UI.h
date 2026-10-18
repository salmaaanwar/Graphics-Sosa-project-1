#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

// Game states shared across modules
enum GameState {
    STATE_MAIN_MENU = 0,
    STATE_CUTSCENE  = 1,
    STATE_PLAYING   = 2,
    STATE_PAUSED    = 3,
    STATE_WON       = 4,
    STATE_LOST      = 5
};

// Player card costs (Melee = 10, Long Range = 40, Defense = 60)
const int MELEE_COST   = 10;
const int RANGED_COST  = 40;
const int DEFENSE_COST = 60;

// Shared UI & placeholder state variables
extern int currentGameState;
extern int collectibles;
extern int selectedCard;

// Placeholder unit health values for testing HUD & health bars
extern int testMeleeHP;
extern int testRangedHP;
extern int testDefenseHP;
extern int testEnemyHP;

// Reset placeholder values (used on Restart / Return to Main Menu)
void resetGamePlaceholders();

// Composite icon drawing helpers (built from multiple primitives)
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

