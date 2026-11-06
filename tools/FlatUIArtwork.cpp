// Offline source artwork; excluded from the running game.
#include "UI.h"
#include "Cutscene.h"
#include "Environment.h"
#include "Player.h"
#include "Renderer.h"
#include "TextRenderer.h"

bool artworkBackdropOnly = false;

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

// simple button drawing
void drawButton(
    GLint transformLocation,
    GLint colorLocation,
    float x,
    float y,
    float width,
    float height,
    const std::string& label,
    float textSize,
    bool isHovered,
    float baseR,
    float baseG,
    float baseB
) {
    if(artworkBackdropOnly) return;
    // outer border
    if (isHovered) {
        glUniform3f(colorLocation, 1.0f, 0.90f, 0.25f);
    } else {
        glUniform3f(colorLocation, 0.22f, 0.25f, 0.30f);
    }
    drawRectangle(transformLocation, x, y, width, height);

    // inner box
    glUniform3f(colorLocation, baseR, baseG, baseB);
    drawRectangle(transformLocation, x, y, width - 0.025f, height - 0.025f);

    // button text
    glUniform3f(colorLocation, 1.0f, 1.0f, 1.0f);
    drawCenteredText(transformLocation, label, x, y, textSize);
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
    // outer coin rim
    setUIColor(colorLocation, 0.95f, 0.75f, 0.15f, 0.45f, grayscale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x, y, 1.0f * scale);

    // inner fill
    setUIColor(colorLocation, 1.0f, 0.88f, 0.30f, 0.60f, grayscale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x, y, 0.78f * scale);

    // center star
    setUIColor(colorLocation, 1.0f, 0.98f, 0.75f, 0.80f, grayscale);
    drawRectangle(transformLocation, x, y, 0.75f * scale, 0.25f * scale, 45.0f);
    drawRectangle(transformLocation, x, y, 0.75f * scale, 0.25f * scale, -45.0f);

    // shine dot
    setUIColor(colorLocation, 1.0f, 1.0f, 1.0f, 0.88f, grayscale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.35f * scale, y + 0.35f * scale, 0.18f * scale);
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
    // handle grip
    setUIColor(colorLocation, 0.25f, 0.25f, 0.30f, 0.32f, grayscale);
    drawRectangle(transformLocation, x, y - 0.55f * scale, 0.16f * scale, 0.75f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x, y - 0.92f * scale, 0.10f * scale);

    // neck connector
    setUIColor(colorLocation, 0.85f, 0.25f, 0.30f, 0.48f, grayscale);
    drawTriangle(transformLocation, triangleVertexStart, x, y - 0.10f * scale, 0.50f * scale, 0.35f * scale, 180.0f);

    // outer head frame
    setUIColor(colorLocation, 0.95f, 0.28f, 0.32f, 0.58f, grayscale);
    drawRectangle(transformLocation, x, y + 0.38f * scale, 0.95f * scale, 0.85f * scale);

    // inner mesh background
    setUIColor(colorLocation, 0.55f, 0.12f, 0.16f, 0.38f, grayscale);
    drawRectangle(transformLocation, x, y + 0.38f * scale, 0.75f * scale, 0.65f * scale);

    // grid lines
    setUIColor(colorLocation, 1.0f, 0.55f, 0.58f, 0.68f, grayscale);
    drawRectangle(transformLocation, x - 0.20f * scale, y + 0.38f * scale, 0.06f * scale, 0.65f * scale);
    drawRectangle(transformLocation, x,                  y + 0.38f * scale, 0.06f * scale, 0.65f * scale);
    drawRectangle(transformLocation, x + 0.20f * scale, y + 0.38f * scale, 0.06f * scale, 0.65f * scale);
    drawRectangle(transformLocation, x, y + 0.18f * scale, 0.75f * scale, 0.06f * scale);
    drawRectangle(transformLocation, x, y + 0.38f * scale, 0.75f * scale, 0.06f * scale);
    drawRectangle(transformLocation, x, y + 0.58f * scale, 0.75f * scale, 0.06f * scale);
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
    // bottle body and shoulder
    setUIColor(colorLocation, 0.18f, 0.68f, 0.95f, 0.48f, grayscale);
    drawRectangle(transformLocation, x - 0.05f * scale, y - 0.38f * scale, 0.78f * scale, 0.88f * scale);
    drawTriangle(transformLocation, triangleVertexStart, x - 0.05f * scale, y + 0.22f * scale, 0.78f * scale, 0.38f * scale);

    // bottle label
    setUIColor(colorLocation, 0.95f, 0.98f, 1.0f, 0.72f, grayscale);
    drawRectangle(transformLocation, x - 0.05f * scale, y - 0.38f * scale, 0.56f * scale, 0.42f * scale);

    // label stripe
    setUIColor(colorLocation, 0.10f, 0.45f, 0.80f, 0.40f, grayscale);
    drawRectangle(transformLocation, x - 0.05f * scale, y - 0.38f * scale, 0.42f * scale, 0.12f * scale);

    // neck collar
    setUIColor(colorLocation, 0.95f, 0.85f, 0.20f, 0.35f, grayscale);
    drawRectangle(transformLocation, x - 0.05f * scale, y + 0.42f * scale, 0.32f * scale, 0.16f * scale);

    // spray head
    setUIColor(colorLocation, 0.95f, 0.30f, 0.30f, 0.62f, grayscale);
    drawRectangle(transformLocation, x + 0.02f * scale, y + 0.62f * scale, 0.62f * scale, 0.24f * scale);

    // trigger lever
    setUIColor(colorLocation, 0.90f, 0.20f, 0.20f, 0.30f, grayscale);
    drawRectangle(transformLocation, x + 0.22f * scale, y + 0.38f * scale, 0.10f * scale, 0.32f * scale, 25.0f);

    // spray droplets
    setUIColor(colorLocation, 0.55f, 0.92f, 1.0f, 0.78f, grayscale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x + 0.48f * scale, y + 0.68f * scale, 0.07f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x + 0.62f * scale, y + 0.55f * scale, 0.06f * scale);
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
    // outer frame
    setUIColor(colorLocation, 0.65f, 0.42f, 0.18f, 0.36f, grayscale);
    drawRectangle(transformLocation, x, y - 0.05f * scale, 0.88f * scale, 1.35f * scale);

    // top roller
    setUIColor(colorLocation, 0.82f, 0.58f, 0.28f, 0.52f, grayscale);
    drawRectangle(transformLocation, x, y + 0.68f * scale, 0.98f * scale, 0.20f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.49f * scale, y + 0.68f * scale, 0.10f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x + 0.49f * scale, y + 0.68f * scale, 0.10f * scale);

    // yellow sticky surface
    setUIColor(colorLocation, 0.98f, 0.88f, 0.22f, 0.65f, grayscale);
    drawRectangle(transformLocation, x, y - 0.08f * scale, 0.68f * scale, 1.08f * scale);

    // glue drops
    setUIColor(colorLocation, 0.92f, 0.65f, 0.10f, 0.48f, grayscale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.15f * scale, y + 0.20f * scale, 0.12f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x + 0.16f * scale, y - 0.05f * scale, 0.14f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.10f * scale, y - 0.35f * scale, 0.11f * scale);

    // defense cross
    setUIColor(colorLocation, 0.35f, 0.80f, 0.30f, 0.78f, grayscale);
    drawRectangle(transformLocation, x, y - 0.08f * scale, 0.42f * scale, 0.12f * scale);
    drawRectangle(transformLocation, x, y - 0.08f * scale, 0.12f * scale, 0.42f * scale);
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
    // wings
    glUniform3f(colorLocation, 0.75f, 0.88f, 0.95f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.10f * scale, y + 0.42f * scale, 0.28f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x + 0.22f * scale, y + 0.45f * scale, 0.26f * scale);

    // body
    glUniform3f(colorLocation, 0.22f, 0.26f, 0.20f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x + 0.25f * scale, y, 0.32f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.12f * scale, y + 0.02f * scale, 0.26f * scale);

    // red eye
    glUniform3f(colorLocation, 0.95f, 0.20f, 0.20f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.28f * scale, y + 0.10f * scale, 0.13f * scale);

    // legs
    glUniform3f(colorLocation, 0.15f, 0.15f, 0.15f);
    drawRectangle(transformLocation, x - 0.10f * scale, y - 0.32f * scale, 0.06f * scale, 0.30f * scale, 20.0f);
    drawRectangle(transformLocation, x + 0.10f * scale, y - 0.32f * scale, 0.06f * scale, 0.30f * scale, 0.0f);
    drawRectangle(transformLocation, x + 0.30f * scale, y - 0.32f * scale, 0.06f * scale, 0.30f * scale, -20.0f);

    // small gun
    glUniform3f(colorLocation, 0.45f, 0.45f, 0.50f);
    drawRectangle(transformLocation, x - 0.48f * scale, y - 0.05f * scale, 0.38f * scale, 0.12f * scale);
    drawRectangle(transformLocation, x - 0.35f * scale, y - 0.16f * scale, 0.10f * scale, 0.18f * scale);
    drawTriangle(transformLocation, triangleVertexStart, x - 0.68f * scale, y - 0.05f * scale, 0.12f * scale, 0.12f * scale, 90.0f);
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
    drawRectangle(transformLocation, x, y, barWidth + 0.012f, barHeight + 0.012f);

    glUniform3f(colorLocation, 0.35f, 0.12f, 0.12f);
    drawRectangle(transformLocation, x, y, barWidth, barHeight);

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
        drawRectangle(transformLocation, fillX, y, fillWidth, barHeight);
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
    // top bar background and bottom trim
    glUniform3f(colorLocation, 0.16f, 0.18f, 0.22f);
    float top = getEnvironment().getViewHalfHeight();
    float screenWidth = getEnvironment().getViewHalfWidth() * 2.0f;
    drawRectangle(transformLocation, 0.0f, (top + 0.68f) * 0.5f,
                  screenWidth, top - 0.68f);

    glUniform3f(colorLocation, 0.78f, 0.65f, 0.42f);
    drawRectangle(transformLocation, 0.0f, 0.68f, screenWidth, 0.016f);

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

        // card outer border
        if (!unlocked) {
            glUniform3f(colorLocation, 0.32f, 0.32f, 0.32f);
        } else if (isSelected) {
            glUniform3f(colorLocation, 0.25f, 1.0f, 0.40f);
        } else if (hovered) {
            glUniform3f(colorLocation, 1.0f, 0.92f, 0.25f);
        } else {
            glUniform3f(colorLocation, 0.85f, 0.72f, 0.45f);
        }
        drawRectangle(transformLocation, cardCentersX[i], cardCenterY, cardW, cardH);

        // card inner background
        if (!unlocked) {
            glUniform3f(colorLocation, 0.22f, 0.22f, 0.22f);
        } else if (hovered || isSelected) {
            glUniform3f(colorLocation, 0.28f, 0.36f, 0.46f);
        } else {
            glUniform3f(colorLocation, 0.20f, 0.26f, 0.34f);
        }
        drawRectangle(transformLocation, cardCentersX[i], cardCenterY, cardW - 0.02f, cardH - 0.02f);

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
        drawRectangle(transformLocation, cardCentersX[i], cardCenterY - 0.092f, cardW - 0.02f, 0.052f);

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

    glUniform3f(colorLocation, 0.85f, 0.72f, 0.35f);
    drawRectangle(transformLocation, coinBoxX, coinBoxY, coinBoxW, coinBoxH);

    glUniform3f(colorLocation, 0.12f, 0.15f, 0.20f);
    drawRectangle(transformLocation, coinBoxX, coinBoxY, coinBoxW - 0.02f, coinBoxH - 0.02f);

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
    drawButton(transformLocation, colorLocation, pauseBtnX, pauseBtnY, pauseBtnW, pauseBtnH,
               "PAUSE", 0.0055f, pauseHovered, 0.28f, 0.35f, 0.48f);
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
    // The shared kitchen image is drawn behind this menu in main.cpp.
    // menu board frame
    glUniform3f(colorLocation, 0.85f, 0.72f, 0.45f);
    drawRectangle(transformLocation, 0.0f, 0.06f, 1.56f, 1.44f);
    glUniform3f(colorLocation, 0.18f, 0.24f, 0.32f);
    drawRectangle(transformLocation, 0.0f, 0.06f, 1.50f, 1.38f);

    // title banner
    glUniform3f(colorLocation, 0.12f, 0.16f, 0.22f);
    drawRectangle(transformLocation, 0.0f, 0.50f, 1.36f, 0.32f);

    glUniform3f(colorLocation, 1.0f, 0.90f, 0.28f);
    drawCenteredText(transformLocation, "KITCHEN DEFENSE", 0.0f, 0.55f, 0.011f);

    glUniform3f(colorLocation, 0.65f, 0.90f, 1.0f);
    drawCenteredText(transformLocation, "DETERGENTS VS INSECTS", 0.0f, 0.41f, 0.0062f);

    // character icons on the sides
    drawSwatterIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                    -0.52f, -0.08f, 0.16f, false);
    drawSprayBottleIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                        -0.52f, -0.45f, 0.13f, false);
    drawMenuInsect(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                   0.52f, -0.18f, 0.18f);

    // start and exit buttons
    float btnW = 0.68f, btnH = 0.20f;
    bool startHovered = isPointInRect(mouseX, mouseY, 0.0f, 0.02f, btnW, btnH);
    drawButton(transformLocation, colorLocation, 0.0f, 0.02f, btnW, btnH,
               "START GAME", 0.0082f, startHovered, 0.18f, 0.62f, 0.30f);

    bool exitHovered = isPointInRect(mouseX, mouseY, 0.0f, -0.28f, btnW, btnH);
    drawButton(transformLocation, colorLocation, 0.0f, -0.28f, btnW, btnH,
               "EXIT GAME", 0.0082f, exitHovered, 0.72f, 0.22f, 0.22f);
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
    // pause box frame
    glUniform3f(colorLocation, 0.85f, 0.72f, 0.42f);
    drawRectangle(transformLocation, 0.0f, -0.05f, 1.12f, 1.20f);

    glUniform3f(colorLocation, 0.16f, 0.20f, 0.28f);
    drawRectangle(transformLocation, 0.0f, -0.05f, 1.06f, 1.14f);

    glUniform3f(colorLocation, 1.0f, 0.92f, 0.30f);
    drawCenteredText(transformLocation, "GAME PAUSED", 0.0f, 0.36f, 0.010f);

    float btnW = 0.78f;
    float btnH = 0.17f;

    bool contHovered = isPointInRect(mouseX, mouseY, 0.0f, 0.10f, btnW, btnH);
    drawButton(transformLocation, colorLocation, 0.0f, 0.10f, btnW, btnH,
               "CONTINUE", 0.0075f, contHovered, 0.20f, 0.62f, 0.32f);

    bool restartHovered = isPointInRect(mouseX, mouseY, 0.0f, -0.14f, btnW, btnH);
    drawButton(transformLocation, colorLocation, 0.0f, -0.14f, btnW, btnH,
               "RESTART", 0.0075f, restartHovered, 0.25f, 0.48f, 0.75f);

    bool menuHovered = isPointInRect(mouseX, mouseY, 0.0f, -0.38f, btnW, btnH);
    drawButton(transformLocation, colorLocation, 0.0f, -0.38f, btnW, btnH,
               "MAIN MENU", 0.0075f, menuHovered, 0.68f, 0.26f, 0.24f);
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
    // center box frame
    if (isWin) {
        glUniform3f(colorLocation, 0.95f, 0.82f, 0.25f);
    } else {
        glUniform3f(colorLocation, 0.85f, 0.25f, 0.25f);
    }
    drawRectangle(transformLocation, 0.0f, 0.0f, 1.32f, 1.32f);

    glUniform3f(colorLocation, 0.14f, 0.18f, 0.24f);
    drawRectangle(transformLocation, 0.0f, 0.0f, 1.26f, 1.26f);

    if (isWin) {
        glUniform3f(colorLocation, 0.30f, 0.95f, 0.45f);
        drawCenteredText(transformLocation, "YOU WIN", 0.0f, 0.44f, 0.013f);

        glUniform3f(colorLocation, 0.85f, 0.95f, 1.0f);
        drawCenteredText(transformLocation, "KITCHEN IS CLEAN", 0.0f, 0.28f, 0.0065f);

        drawCollectibleIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount,
                            -0.46f, 0.38f, 0.09f, false);
        drawCollectibleIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount,
                            0.46f, 0.38f, 0.09f, false);
    } else {
        glUniform3f(colorLocation, 1.0f, 0.28f, 0.28f);
        drawCenteredText(transformLocation, "YOU LOSE", 0.0f, 0.44f, 0.013f);

        glUniform3f(colorLocation, 1.0f, 0.82f, 0.82f);
        drawCenteredText(transformLocation, "INSECTS REACHED THE FOOD", 0.0f, 0.28f, 0.0055f);

        drawMenuInsect(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                       -0.48f, 0.42f, 0.12f);
        drawMenuInsect(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                       0.48f, 0.42f, 0.12f);
    }

    float btnW = 0.78f;
    float btnH = 0.18f;

    bool restartHovered = isPointInRect(mouseX, mouseY, 0.0f, -0.04f, btnW, btnH);
    drawButton(transformLocation, colorLocation, 0.0f, -0.04f, btnW, btnH,
               "RESTART", 0.0080f, restartHovered, 0.20f, 0.60f, 0.32f);

    bool menuHovered = isPointInRect(mouseX, mouseY, 0.0f, -0.32f, btnW, btnH);
    drawButton(transformLocation, colorLocation, 0.0f, -0.32f, btnW, btnH,
               "MAIN MENU", 0.0080f, menuHovered, 0.68f, 0.26f, 0.24f);
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
    if (getGameState() == STATE_MAIN_MENU) {
        // start button
        if (isPointInRect(mouseX, mouseY, 0.0f, 0.02f, 0.68f, 0.20f)) {
            startNewGame();
            return;
        }
        // exit button
        if (isPointInRect(mouseX, mouseY, 0.0f, -0.28f, 0.68f, 0.20f)) {
            glfwSetWindowShouldClose(window, true);
            return;
        }
    }
    else if (getGameState() == STATE_CUTSCENE) {
        if (isPointInRect(mouseX, mouseY, SKIP_BUTTON_X, SKIP_BUTTON_Y,
                          SKIP_BUTTON_WIDTH, SKIP_BUTTON_HEIGHT)) {
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
            restartGame();
            return;
        }
        // main menu
        if (isPointInRect(mouseX, mouseY, 0.0f, -0.38f, btnW, btnH)) {
            returnToMainMenu();
            return;
        }
    }
    else if (getGameState() == STATE_WON || getGameState() == STATE_LOST) {
        float btnW = 0.78f;
        float btnH = 0.18f;

        // restart
        if (isPointInRect(mouseX, mouseY, 0.0f, -0.04f, btnW, btnH)) {
            restartGame();
            return;
        }
        // main menu
        if (isPointInRect(mouseX, mouseY, 0.0f, -0.32f, btnW, btnH)) {
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
