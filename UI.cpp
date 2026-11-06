#include "Audio.h"
#include "UI.h"
#include "Cutscene.h"
#include "Environment.h"
#include "Player.h"
#include "Renderer.h"
#include "TextRenderer.h"
#include <algorithm>

// Drawing and hit testing share the same positions on a wider counter.
static float cardCenterX(int index)
{
    return -getEnvironment().getViewHalfWidth() + 0.26f + index * 0.30f;
}

static float pauseCenterX()
{
    return getEnvironment().getViewHalfWidth() - 0.22f;
}

static const float SKIP_BUTTON_X = 0.80f;
static const float SKIP_BUTTON_Y = 0.86f;
static const float SKIP_BUTTON_WIDTH = 0.28f;
static const float SKIP_BUTTON_HEIGHT = 0.14f;

// check if point (px, py) is inside rectangle centered at (rx, ry)
bool isPointInRect(float px, float py, float rx, float ry, float rw, float rh)
{
    float halfW = rw * 0.5f;
    float halfH = rh * 0.5f;
    return (px >= rx - halfW && px <= rx + halfW &&
            py >= ry - halfH && py <= ry + halfH);
}

// draw text centered around (centerX, centerY)
void drawCenteredText(
    GLint transformLocation,
    const std::string& text,
    float centerX,
    float centerY,
    float cellSize
) {
    if (text.empty()) {
        return;
    }
    float totalWidth = (6.0f * (float)text.length() - 1.0f) * cellSize;
    float totalHeight = 7.0f * cellSize;
    float startX = centerX - totalWidth * 0.5f;
    float startY = centerY + totalHeight * 0.5f;
    drawText(transformLocation, text, startX, startY, cellSize);
}

// set normal color or grayscale color if locked
void setUIColor(GLint colorLocation, float r, float g, float b, float gray, bool grayscale)
{
    if (grayscale) {
        glUniform3f(colorLocation, gray, gray, gray);
    } else {
        glUniform3f(colorLocation, r, g, b);
    }
}

// Buttons are complete images, including their labels.
static void drawMenuButton(GLint transformLocation, GLint colorLocation,
                           float x, float y, float width, float height,
                           const std::string& label, bool isHovered)
{
    std::string key=label;
    std::replace(key.begin(),key.end(),' ','_');
    drawIntroImage(transformLocation,colorLocation,"button-"+key+(isHovered ? "-1" : "-0"),x,y,width,height);
}

// collectible coin icon
void drawCollectibleIcon(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    float x,
    float y,
    float scale,
    bool grayscale
) {
    float size=2*scale/.8f;
    drawIntroImage(transformLocation,colorLocation,"icon-coin-"+std::to_string(grayscale ? 1 : 0),x,y,size,size);
}

// melee fly swatter icon
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
) {
    float size=2*scale/.65f;
    drawIntroImage(transformLocation,colorLocation,"icon-swatter-"+std::to_string(grayscale ? 1 : 0),x,y,size,size);
}

// long range spray bottle icon
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
) {
    float size=2*scale/.65f;
    drawIntroImage(transformLocation,colorLocation,"icon-spray-"+std::to_string(grayscale ? 1 : 0),x,y,size,size);
}

// defense sticky sheet icon
void drawStickySheetIcon(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    float x,
    float y,
    float scale,
    bool grayscale
) {
    float size=2*scale/.65f;
    drawIntroImage(transformLocation,colorLocation,"icon-sticky-"+std::to_string(grayscale ? 1 : 0),x,y,size,size);
}

// insect icon for menus
void drawMenuInsect(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float x,
    float y,
    float scale
) {
    float size=2*scale/.65f;
    drawIntroImage(transformLocation,colorLocation,"icon-insect",x,y,size,size);
}

// health bar drawing
void drawHealthBar(
    GLint transformLocation,
    GLint colorLocation,
    float x,
    float y,
    int currentHP,
    int maxHP
) {
    if (maxHP <= 0) {
        return;
    }

    int clampedHP = currentHP;
    if (clampedHP < 0) clampedHP = 0;
    if (clampedHP > maxHP) clampedHP = maxHP;

    float barWidth = 0.18f;
    float barHeight = 0.032f;

    // dark outer frame and background
    glUniform3f(colorLocation, 0.10f, 0.10f, 0.12f);
    drawPictureTinted(transformLocation, "ui-fill", x, y, barWidth + 0.012f, barHeight + 0.012f);

    glUniform3f(colorLocation, 0.35f, 0.12f, 0.12f);
    drawPictureTinted(transformLocation, "ui-fill", x, y, barWidth, barHeight);

    // filled health bar
    float ratio = (float)clampedHP / (float)maxHP;
    if (ratio > 0.0f) {
        float fillWidth = barWidth * ratio;
        float fillX = (x - barWidth * 0.5f) + fillWidth * 0.5f;

        if (ratio > 0.55f) {
            glUniform3f(colorLocation, 0.20f, 0.88f, 0.32f);
        } else if (ratio > 0.28f) {
            glUniform3f(colorLocation, 0.95f, 0.80f, 0.18f);
        } else {
            glUniform3f(colorLocation, 0.95f, 0.22f, 0.20f);
        }
        drawPictureTinted(transformLocation, "ui-fill", fillX, y, fillWidth, barHeight);
    }

    // numeric HP next to bar
    glUniform3f(colorLocation, 1.0f, 1.0f, 1.0f);
    drawNumber(transformLocation, clampedHP, x + barWidth * 0.5f + 0.015f, y + 0.016f, 0.0045f);
}

// top HUD drawing
void drawHUD(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float mouseX,
    float mouseY
) {
    float top=getEnvironment().getViewHalfHeight();
    float screenWidth=getEnvironment().getViewHalfWidth()*2;
    drawIntroImage(transformLocation,colorLocation,"hud-bar",0,(top+.68f)*.5f,screenWidth,top-.68f);

    // 3 player cards on the left
    const int costs[3] = { MELEE_COST, RANGED_COST, DEFENSE_COST };
    const char* cardLabels[3] = { "SWATTER", "SPRAY", "STICKY" };
    const float cardCentersX[3] = { cardCenterX(0), cardCenterX(1), cardCenterX(2) };
    const float cardCenterY = 0.84f;
    const float cardW = 0.25f;
    const float cardH = 0.26f;

    for (int i = 0; i < 3; i++) {
        bool unlocked = isCardUnlocked(i);
        bool hovered = false;

        if (unlocked) {
            hovered = isPointInRect(mouseX, mouseY, cardCentersX[i], cardCenterY, cardW, cardH);
        }

        bool isSelected = (unlocked && getSelectedCard() == i);

        int cardState=!unlocked ? 1 : isSelected ? 3 : hovered ? 2 : 0;
        drawIntroImage(transformLocation,colorLocation,"hud-card-"+std::to_string(cardState),
                       cardCentersX[i],cardCenterY,cardW,cardH);

        // card name
        setUIColor(colorLocation, 1.0f, 1.0f, 1.0f, 0.55f, !unlocked);
        drawCenteredText(transformLocation, cardLabels[i], cardCentersX[i], cardCenterY + 0.095f, 0.0042f);

        // card icon
        int dropsLeft = getRemainingUnlockDrops(i);
        float iconY = cardCenterY + (dropsLeft > 0 ? 0.016f : 0.005f);
        float iconScale = dropsLeft > 0 ? 0.060f : 0.075f;
        if (i == 0) {
            drawSwatterIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                            cardCentersX[i], iconY, iconScale, !unlocked);
        } else if (i == 1) {
            drawSprayBottleIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                                cardCentersX[i], iconY, iconScale, !unlocked);
        } else {
            drawStickySheetIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount,
                                cardCentersX[i], iconY, dropsLeft > 0 ? iconScale : 0.072f, !unlocked);
        }

        if (dropsLeft > 0) {
            std::string hint = std::to_string(dropsLeft) + (dropsLeft == 1 ? " DROP LEFT" : " DROPS LEFT");
            glUniform3f(colorLocation, 0.72f, 0.72f, 0.72f);
            drawCenteredText(transformLocation, hint, cardCentersX[i], cardCenterY - 0.050f, 0.0032f);
        }

        // bottom cost strip
        setUIColor(colorLocation, 0.12f, 0.16f, 0.22f, 0.16f, !unlocked);
        drawPictureTinted(transformLocation, "ui-fill", cardCentersX[i], cardCenterY - 0.092f, cardW - 0.02f, 0.052f);

        drawCollectibleIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount,
                            cardCentersX[i] - 0.055f, cardCenterY - 0.092f, 0.020f, !unlocked);

        setUIColor(colorLocation, 1.0f, 0.92f, 0.35f, 0.58f, !unlocked);
        drawNumber(transformLocation, costs[i], cardCentersX[i] - 0.015f, cardCenterY - 0.074f, 0.0052f);
    }

    // coins counter box on the right
    float coinBoxX = getEnvironment().getViewHalfWidth() - 0.66f;
    float coinBoxY = 0.84f;
    float coinBoxW = 0.44f;
    float coinBoxH = 0.18f;

    drawIntroImage(transformLocation,colorLocation,"hud-coins",coinBoxX,coinBoxY,coinBoxW,coinBoxH);

    drawCollectibleIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount,
                        coinBoxX - 0.13f, coinBoxY, 0.055f, false);

    glUniform3f(colorLocation, 0.80f, 0.90f, 1.0f);
    drawText(transformLocation, "COINS", coinBoxX - 0.04f, coinBoxY + 0.062f, 0.0048f);

    glUniform3f(colorLocation, 1.0f, 0.92f, 0.28f);
    drawNumber(transformLocation, getCollectibles(), coinBoxX - 0.04f, coinBoxY + 0.012f, 0.0085f);

    // pause button on far right
    float pauseBtnX = pauseCenterX();
    float pauseBtnY = 0.84f;
    float pauseBtnW = 0.26f;
    float pauseBtnH = 0.14f;
    bool pauseHovered = isPointInRect(mouseX, mouseY, pauseBtnX, pauseBtnY, pauseBtnW, pauseBtnH);
    drawMenuButton(transformLocation, colorLocation, pauseBtnX, pauseBtnY, pauseBtnW, pauseBtnH, "PAUSE", pauseHovered);
}

// main menu drawing
void drawMainMenu(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float mouseX,
    float mouseY
) {
    drawIntroImage(transformLocation,colorLocation,"menu-main",0,0,2,2);
    drawAudioControls(transformLocation, colorLocation);
    // start and exit buttons
    float btnW = 0.68f, btnH = 0.20f;
    bool startHovered = isPointInRect(mouseX, mouseY, 0.0f, 0.02f, btnW, btnH);
    drawMenuButton(transformLocation, colorLocation, 0.0f, 0.02f, btnW, btnH, "START GAME", startHovered);

    bool exitHovered = isPointInRect(mouseX, mouseY, 0.0f, -0.28f, btnW, btnH);
    drawMenuButton(transformLocation, colorLocation, 0.0f, -0.28f, btnW, btnH, "EXIT GAME", exitHovered);
}

void drawCutsceneSkipButton(GLint transformLocation, GLint colorLocation, float mouseX, float mouseY)
{
    bool hovered = isPointInRect(mouseX, mouseY, SKIP_BUTTON_X, SKIP_BUTTON_Y,
                                 SKIP_BUTTON_WIDTH, SKIP_BUTTON_HEIGHT);
    drawIntroImage(transformLocation,colorLocation,hovered ? "intro-skip-1" : "intro-skip-0",
                   SKIP_BUTTON_X,SKIP_BUTTON_Y,SKIP_BUTTON_WIDTH,SKIP_BUTTON_HEIGHT);
}

// pause menu drawing
void drawPauseMenu(
    GLint transformLocation,
    GLint colorLocation,
    float mouseX,
    float mouseY
) {
    drawIntroImage(transformLocation,colorLocation,"menu-pause",0,0,2,2);
    drawAudioControls(transformLocation, colorLocation);
    float btnW = 0.78f;
    float btnH = 0.17f;

    bool contHovered = isPointInRect(mouseX, mouseY, 0.0f, 0.10f, btnW, btnH);
    drawMenuButton(transformLocation, colorLocation, 0.0f, 0.10f, btnW, btnH, "CONTINUE", contHovered);

    bool restartHovered = isPointInRect(mouseX, mouseY, 0.0f, -0.14f, btnW, btnH);
    drawMenuButton(transformLocation, colorLocation, 0.0f, -0.14f, btnW, btnH, "RESTART", restartHovered);

    bool menuHovered = isPointInRect(mouseX, mouseY, 0.0f, -0.38f, btnW, btnH);
    drawMenuButton(transformLocation, colorLocation, 0.0f, -0.38f, btnW, btnH, "MAIN MENU", menuHovered);
}

// win / lose menu drawing
void drawWinLoseMenu(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    bool isWin,
    float mouseX,
    float mouseY
) {
    drawIntroImage(transformLocation,colorLocation,isWin ? "menu-win" : "menu-lose",0,0,2,2);
    float btnW = 0.78f;
    float btnH = 0.18f;

    bool restartHovered = isPointInRect(mouseX, mouseY, 0.0f, -0.04f, btnW, btnH);
    drawMenuButton(transformLocation, colorLocation, 0.0f, -0.04f, btnW, btnH, "RESTART", restartHovered);

    bool menuHovered = isPointInRect(mouseX, mouseY, 0.0f, -0.32f, btnW, btnH);
    drawMenuButton(transformLocation, colorLocation, 0.0f, -0.32f, btnW, btnH, "MAIN MENU", menuHovered);
}

// UI fallback drawing (used for main menu)
void drawUI(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float mouseX,
    float mouseY
) {
    if (getGameState() == STATE_MAIN_MENU) {
        drawMainMenu(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart, mouseX, mouseY);
    }
}

// UI mouse click handling
void handleUIMouseClick(
    GLFWwindow* window,
    float mouseX,
    float mouseY
) {
    if (handleAudioControlsMouse(mouseX, mouseY, GLFW_PRESS)) return;
    if (getGameState() == STATE_MAIN_MENU) {
        // start button
        if (isPointInRect(mouseX, mouseY, 0.0f, 0.02f, 0.68f, 0.20f)) {
            playSound(SoundId::Button);
            startNewGame();
            return;
        }
        // exit button
        if (isPointInRect(mouseX, mouseY, 0.0f, -0.28f, 0.68f, 0.20f)) {
            playSound(SoundId::Button);
            glfwSetWindowShouldClose(window, true);
            return;
        }
    }
    else if (getGameState() == STATE_CUTSCENE) {
        if (isPointInRect(mouseX, mouseY, SKIP_BUTTON_X, SKIP_BUTTON_Y,
                          SKIP_BUTTON_WIDTH, SKIP_BUTTON_HEIGHT)) {
            playSound(SoundId::Button);
            updateCutscene(CUTSCENE_DURATION);
            setGameState(STATE_PLAYING);
        }
    }
    else if (getGameState() == STATE_PLAYING) {
        // 3 player cards in HUD
        const float cardCentersX[3] = { cardCenterX(0), cardCenterX(1), cardCenterX(2) };
        const float cardCenterY = 0.84f;
        const float cardW = 0.25f;
        const float cardH = 0.26f;

        for (int i = 0; i < 3; i++) {
            if (isPointInRect(mouseX, mouseY, cardCentersX[i], cardCenterY, cardW, cardH)) {
                selectPlayerCard(i);
                return;
            }
        }

        // pause button on top right
        if (isPointInRect(mouseX, mouseY, pauseCenterX(), 0.84f, 0.26f, 0.14f)) {
            pauseGame();
            return;
        }
    }
    else if (getGameState() == STATE_PAUSED) {
        float btnW = 0.78f;
        float btnH = 0.17f;

        // continue
        if (isPointInRect(mouseX, mouseY, 0.0f, 0.10f, btnW, btnH)) {
            resumeGame();
            return;
        }
        // restart
        if (isPointInRect(mouseX, mouseY, 0.0f, -0.14f, btnW, btnH)) {
            playSound(SoundId::Button);
            restartGame();
            return;
        }
        // main menu
        if (isPointInRect(mouseX, mouseY, 0.0f, -0.38f, btnW, btnH)) {
            playSound(SoundId::Button);
            returnToMainMenu();
            return;
        }
    }
    else if (getGameState() == STATE_WON || getGameState() == STATE_LOST) {
        float btnW = 0.78f;
        float btnH = 0.18f;

        // restart
        if (isPointInRect(mouseX, mouseY, 0.0f, -0.04f, btnW, btnH)) {
            playSound(SoundId::Button);
            restartGame();
            return;
        }
        // main menu
        if (isPointInRect(mouseX, mouseY, 0.0f, -0.32f, btnW, btnH)) {
            playSound(SoundId::Button);
            returnToMainMenu();
            return;
        }
    }
}

// UI keyboard input handling
void handleUIKeyPress(
    GLFWwindow* window,
    int key,
    int action
) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) {
        return;
    }

    if (handleAudioControlsKey(key, action)) return;

    // ESC toggles pause during gameplay
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        if (getGameState() == STATE_PLAYING) {
            pauseGame();
        } else if (getGameState() == STATE_PAUSED) {
            resumeGame();
        }
        return;
    }

    if (!isGameplayRunning()) return;

    // W/Up and S/Down are gameplay poses, never currency shortcuts.
    handlePlayerKeyPress(key, action);
    (void)window;
}
