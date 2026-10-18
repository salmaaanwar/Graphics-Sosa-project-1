#include "UI.h"
#include "Renderer.h"
#include "TextRenderer.h"
#include <iostream>

// Shared state variables
int currentGameState = STATE_MAIN_MENU;
int collectibles = 30; // Starts with 30 coins (enough for 3 Melee players at cost 10)
int selectedCard = -1; // -1 = none, 0 = Melee, 1 = Ranged, 2 = Defense

// Placeholder unit health values for testing in STATE_PLAYING
int testMeleeHP   = 5; // Melee max HP = 5
int testRangedHP  = 4; // Ranged max HP = 5
int testDefenseHP = 7; // Defense max HP = 7
int testEnemyHP   = 3; // Enemy max HP = 5

int getGameState()
{
    return currentGameState;
}

void setGameState(int state)
{
    currentGameState = state;
}

int getCollectibles()
{
    return collectibles;
}

void setCollectibles(int amount)
{
    collectibles = amount;
    if (collectibles < 0) {
        collectibles = 0;
    }
}

int getSelectedCard()
{
    return selectedCard;
}

// Helper: check if mouse world position (px, py) is inside a rectangle centered at (rx, ry)
bool isPointInRect(float px, float py, float rx, float ry, float rw, float rh)
{
    float halfW = rw * 0.5f;
    float halfH = rh * 0.5f;
    return (px >= rx - halfW && px <= rx + halfW &&
            py >= ry - halfH && py <= ry + halfH);
}

// Helper: draw centered text around (centerX, centerY)
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

// Helper: check if a player card is currently unlocked (depends purely on collectibles >= cost)
bool isCardUnlocked(int cardIndex)
{
    if (cardIndex == 0) {
        return collectibles >= MELEE_COST;
    }
    if (cardIndex == 1) {
        return collectibles >= RANGED_COST;
    }
    if (cardIndex == 2) {
        return collectibles >= DEFENSE_COST;
    }
    return false;
}

// Reset placeholder values on Restart or Return to Main Menu
void resetGamePlaceholders()
{
    collectibles = 30;
    selectedCard = -1;
    testMeleeHP = 5;
    testRangedHP = 4;
    testDefenseHP = 7;
    testEnemyHP = 3;
}

// Helper: draw an interactive layered button with hover highlight
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
    // Drop shadow
    glUniform3f(colorLocation, 0.08f, 0.08f, 0.10f);
    drawRectangle(transformLocation, x + 0.01f, y - 0.01f, width, height);

    // Outer border (bright yellow/gold when hovered, dark trim otherwise)
    if (isHovered) {
        glUniform3f(colorLocation, 1.0f, 0.90f, 0.25f);
    } else {
        glUniform3f(colorLocation, 0.22f, 0.25f, 0.30f);
    }
    drawRectangle(transformLocation, x, y, width, height);

    // Inner fill (brightened when hovered)
    float fillR = baseR;
    float fillG = baseG;
    float fillB = baseB;
    if (isHovered) {
        fillR += 0.18f;
        fillG += 0.18f;
        fillB += 0.18f;
        if (fillR > 1.0f) fillR = 1.0f;
        if (fillG > 1.0f) fillG = 1.0f;
        if (fillB > 1.0f) fillB = 1.0f;
    }
    glUniform3f(colorLocation, fillR, fillG, fillB);
    drawRectangle(transformLocation, x, y, width - 0.025f, height - 0.025f);

    // Decorative side rivets on button (composite detail)
    glUniform3f(colorLocation, 0.85f, 0.85f, 0.90f);
    drawRectangle(transformLocation, x - width * 0.42f, y, 0.015f, height * 0.45f);
    drawRectangle(transformLocation, x + width * 0.42f, y, 0.015f, height * 0.45f);

    // Button label text
    if (isHovered) {
        glUniform3f(colorLocation, 1.0f, 1.0f, 0.85f);
    } else {
        glUniform3f(colorLocation, 1.0f, 1.0f, 1.0f);
    }
    drawCenteredText(transformLocation, label, x, y, textSize);
}

// ============================================================================
// COMPOSITE ICONS (Multi-primitive shapes for Collectible & 3 Player Types)
// ============================================================================

// Composite Collectible Icon: Soapy Kitchen Bubble Coin (Circles + Rotated Star)
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
    // Outer coin rim
    if (grayscale) {
        glUniform3f(colorLocation, 0.45f, 0.45f, 0.45f);
    } else {
        glUniform3f(colorLocation, 0.95f, 0.75f, 0.15f);
    }
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x, y, 1.0f * scale);

    // Inner soapy cyan/gold fill
    if (grayscale) {
        glUniform3f(colorLocation, 0.60f, 0.60f, 0.60f);
    } else {
        glUniform3f(colorLocation, 1.0f, 0.88f, 0.30f);
    }
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x, y, 0.78f * scale);

    // Center diamond sparkle (two rotated rectangles)
    if (grayscale) {
        glUniform3f(colorLocation, 0.80f, 0.80f, 0.80f);
    } else {
        glUniform3f(colorLocation, 1.0f, 0.98f, 0.75f);
    }
    drawRectangle(transformLocation, x, y, 0.75f * scale, 0.25f * scale, 45.0f);
    drawRectangle(transformLocation, x, y, 0.75f * scale, 0.25f * scale, -45.0f);

    // Top-left bubble shine dot
    if (grayscale) {
        glUniform3f(colorLocation, 0.88f, 0.88f, 0.88f);
    } else {
        glUniform3f(colorLocation, 1.0f, 1.0f, 1.0f);
    }
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.35f * scale, y + 0.35f * scale, 0.18f * scale);
}

// Composite Player 1 Icon: Melee Fly Swatter
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
    // 1. Handle grip at bottom
    if (grayscale) {
        glUniform3f(colorLocation, 0.32f, 0.32f, 0.32f);
    } else {
        glUniform3f(colorLocation, 0.25f, 0.25f, 0.30f);
    }
    drawRectangle(transformLocation, x, y - 0.55f * scale, 0.16f * scale, 0.75f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x, y - 0.92f * scale, 0.10f * scale);

    // 2. Flared neck connector (triangle)
    if (grayscale) {
        glUniform3f(colorLocation, 0.48f, 0.48f, 0.48f);
    } else {
        glUniform3f(colorLocation, 0.85f, 0.25f, 0.30f);
    }
    drawTriangle(transformLocation, triangleVertexStart, x, y - 0.10f * scale, 0.50f * scale, 0.35f * scale, 180.0f);

    // 3. Swatter head outer frame
    if (grayscale) {
        glUniform3f(colorLocation, 0.58f, 0.58f, 0.58f);
    } else {
        glUniform3f(colorLocation, 0.95f, 0.28f, 0.32f);
    }
    drawRectangle(transformLocation, x, y + 0.38f * scale, 0.95f * scale, 0.85f * scale);

    // 4. Swatter inner mesh background
    if (grayscale) {
        glUniform3f(colorLocation, 0.38f, 0.38f, 0.38f);
    } else {
        glUniform3f(colorLocation, 0.55f, 0.12f, 0.16f);
    }
    drawRectangle(transformLocation, x, y + 0.38f * scale, 0.75f * scale, 0.65f * scale);

    // 5. Swatter grid lines (vertical and horizontal bars)
    if (grayscale) {
        glUniform3f(colorLocation, 0.68f, 0.68f, 0.68f);
    } else {
        glUniform3f(colorLocation, 1.0f, 0.55f, 0.58f);
    }
    drawRectangle(transformLocation, x - 0.20f * scale, y + 0.38f * scale, 0.06f * scale, 0.65f * scale);
    drawRectangle(transformLocation, x,                  y + 0.38f * scale, 0.06f * scale, 0.65f * scale);
    drawRectangle(transformLocation, x + 0.20f * scale, y + 0.38f * scale, 0.06f * scale, 0.65f * scale);
    drawRectangle(transformLocation, x, y + 0.18f * scale, 0.75f * scale, 0.06f * scale);
    drawRectangle(transformLocation, x, y + 0.38f * scale, 0.75f * scale, 0.06f * scale);
    drawRectangle(transformLocation, x, y + 0.58f * scale, 0.75f * scale, 0.06f * scale);
}

// Composite Player 2 Icon: Long Range Spray Cleaning Bottle
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
    // 1. Bottle main body (bottom rectangle)
    if (grayscale) {
        glUniform3f(colorLocation, 0.48f, 0.48f, 0.48f);
    } else {
        glUniform3f(colorLocation, 0.18f, 0.68f, 0.95f);
    }
    drawRectangle(transformLocation, x - 0.05f * scale, y - 0.38f * scale, 0.78f * scale, 0.88f * scale);

    // 2. Bottle shoulder taper (triangle)
    drawTriangle(transformLocation, triangleVertexStart, x - 0.05f * scale, y + 0.22f * scale, 0.78f * scale, 0.38f * scale);

    // 3. Bottle label on body
    if (grayscale) {
        glUniform3f(colorLocation, 0.72f, 0.72f, 0.72f);
    } else {
        glUniform3f(colorLocation, 0.95f, 0.98f, 1.0f);
    }
    drawRectangle(transformLocation, x - 0.05f * scale, y - 0.38f * scale, 0.56f * scale, 0.42f * scale);

    // 4. Label stripe
    if (grayscale) {
        glUniform3f(colorLocation, 0.40f, 0.40f, 0.40f);
    } else {
        glUniform3f(colorLocation, 0.10f, 0.45f, 0.80f);
    }
    drawRectangle(transformLocation, x - 0.05f * scale, y - 0.38f * scale, 0.42f * scale, 0.12f * scale);

    // 5. Neck collar
    if (grayscale) {
        glUniform3f(colorLocation, 0.35f, 0.35f, 0.35f);
    } else {
        glUniform3f(colorLocation, 0.95f, 0.85f, 0.20f);
    }
    drawRectangle(transformLocation, x - 0.05f * scale, y + 0.42f * scale, 0.32f * scale, 0.16f * scale);

    // 6. Spray head & nozzle
    if (grayscale) {
        glUniform3f(colorLocation, 0.62f, 0.62f, 0.62f);
    } else {
        glUniform3f(colorLocation, 0.95f, 0.30f, 0.30f);
    }
    drawRectangle(transformLocation, x + 0.02f * scale, y + 0.62f * scale, 0.62f * scale, 0.24f * scale);

    // 7. Trigger lever (angled rectangle)
    if (grayscale) {
        glUniform3f(colorLocation, 0.30f, 0.30f, 0.30f);
    } else {
        glUniform3f(colorLocation, 0.90f, 0.20f, 0.20f);
    }
    drawRectangle(transformLocation, x + 0.22f * scale, y + 0.38f * scale, 0.10f * scale, 0.32f * scale, 25.0f);

    // 8. Spray mist droplets in front
    if (grayscale) {
        glUniform3f(colorLocation, 0.78f, 0.78f, 0.78f);
    } else {
        glUniform3f(colorLocation, 0.55f, 0.92f, 1.0f);
    }
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x + 0.48f * scale, y + 0.68f * scale, 0.07f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x + 0.62f * scale, y + 0.55f * scale, 0.06f * scale);
}

// Composite Player 3 Icon: Defense Sticky Sheet Trap
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
    // 1. Outer cardboard frame
    if (grayscale) {
        glUniform3f(colorLocation, 0.36f, 0.36f, 0.36f);
    } else {
        glUniform3f(colorLocation, 0.65f, 0.42f, 0.18f);
    }
    drawRectangle(transformLocation, x, y - 0.05f * scale, 0.88f * scale, 1.35f * scale);

    // 2. Top hanging roller cylinder
    if (grayscale) {
        glUniform3f(colorLocation, 0.52f, 0.52f, 0.52f);
    } else {
        glUniform3f(colorLocation, 0.82f, 0.58f, 0.28f);
    }
    drawRectangle(transformLocation, x, y + 0.68f * scale, 0.98f * scale, 0.20f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.49f * scale, y + 0.68f * scale, 0.10f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x + 0.49f * scale, y + 0.68f * scale, 0.10f * scale);

    // 3. Inner bright yellow sticky glue surface
    if (grayscale) {
        glUniform3f(colorLocation, 0.65f, 0.65f, 0.65f);
    } else {
        glUniform3f(colorLocation, 0.98f, 0.88f, 0.22f);
    }
    drawRectangle(transformLocation, x, y - 0.08f * scale, 0.68f * scale, 1.08f * scale);

    // 4. Glue wave / honey droplets on sticky pad
    if (grayscale) {
        glUniform3f(colorLocation, 0.48f, 0.48f, 0.48f);
    } else {
        glUniform3f(colorLocation, 0.92f, 0.65f, 0.10f);
    }
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.15f * scale, y + 0.20f * scale, 0.12f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x + 0.16f * scale, y - 0.05f * scale, 0.14f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.10f * scale, y - 0.35f * scale, 0.11f * scale);

    // 5. Shield cross bar in center to signify Defense role
    if (grayscale) {
        glUniform3f(colorLocation, 0.78f, 0.78f, 0.78f);
    } else {
        glUniform3f(colorLocation, 0.35f, 0.80f, 0.30f);
    }
    drawRectangle(transformLocation, x, y - 0.08f * scale, 0.42f * scale, 0.12f * scale);
    drawRectangle(transformLocation, x, y - 0.08f * scale, 0.12f * scale, 0.42f * scale);
}

// Composite Enemy Icon (Insect / Fly with Gun) for placeholder scene & menus
void drawPlaceholderInsect(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float x,
    float y,
    float scale
) {
    // Wings (two angled ellipses/circles at top)
    glUniform3f(colorLocation, 0.75f, 0.88f, 0.95f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.10f * scale, y + 0.42f * scale, 0.28f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x + 0.22f * scale, y + 0.45f * scale, 0.26f * scale);

    // Thorax & Abdomen
    glUniform3f(colorLocation, 0.22f, 0.26f, 0.20f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x + 0.25f * scale, y, 0.32f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.12f * scale, y + 0.02f * scale, 0.26f * scale);

    // Red compound eye (facing left toward Home Base)
    glUniform3f(colorLocation, 0.95f, 0.20f, 0.20f);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, x - 0.28f * scale, y + 0.10f * scale, 0.13f * scale);

    // Legs
    glUniform3f(colorLocation, 0.15f, 0.15f, 0.15f);
    drawRectangle(transformLocation, x - 0.10f * scale, y - 0.32f * scale, 0.06f * scale, 0.30f * scale, 20.0f);
    drawRectangle(transformLocation, x + 0.10f * scale, y - 0.32f * scale, 0.06f * scale, 0.30f * scale, 0.0f);
    drawRectangle(transformLocation, x + 0.30f * scale, y - 0.32f * scale, 0.06f * scale, 0.30f * scale, -20.0f);

    // Tiny blaster gun held in front
    glUniform3f(colorLocation, 0.45f, 0.45f, 0.50f);
    drawRectangle(transformLocation, x - 0.48f * scale, y - 0.05f * scale, 0.38f * scale, 0.12f * scale);
    drawRectangle(transformLocation, x - 0.35f * scale, y - 0.16f * scale, 0.10f * scale, 0.18f * scale);
    drawTriangle(transformLocation, triangleVertexStart, x - 0.68f * scale, y - 0.05f * scale, 0.12f * scale, 0.12f * scale, 90.0f);
}

// ============================================================================
// REQUIREMENT 5: REUSABLE UNIT HEALTH DISPLAY COMPONENT
// ============================================================================

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

    // 1. Outer dark frame
    glUniform3f(colorLocation, 0.10f, 0.10f, 0.12f);
    drawRectangle(transformLocation, x, y, barWidth + 0.012f, barHeight + 0.012f);

    // 2. Empty health bar background (dark red/grey)
    glUniform3f(colorLocation, 0.35f, 0.12f, 0.12f);
    drawRectangle(transformLocation, x, y, barWidth, barHeight);

    // 3. Filled health bar proportional to remaining HP
    float ratio = (float)clampedHP / (float)maxHP;
    if (ratio > 0.0f) {
        float fillWidth = barWidth * ratio;
        float fillX = (x - barWidth * 0.5f) + fillWidth * 0.5f;

        // Color shifts from green (high) -> yellow (medium) -> red (low)
        if (ratio > 0.55f) {
            glUniform3f(colorLocation, 0.20f, 0.88f, 0.32f);
        } else if (ratio > 0.28f) {
            glUniform3f(colorLocation, 0.95f, 0.80f, 0.18f);
        } else {
            glUniform3f(colorLocation, 0.95f, 0.22f, 0.20f);
        }
        drawRectangle(transformLocation, fillX, y, fillWidth, barHeight);
    }

    // 4. Segment divider ticks for each life
    glUniform3f(colorLocation, 0.12f, 0.12f, 0.15f);
    for (int i = 1; i < maxHP; i++) {
        float tickX = (x - barWidth * 0.5f) + barWidth * ((float)i / (float)maxHP);
        drawRectangle(transformLocation, tickX, y, 0.004f, barHeight);
    }

    // 5. Numeric HP indicator right next to the bar
    glUniform3f(colorLocation, 1.0f, 1.0f, 1.0f);
    drawNumber(transformLocation, clampedHP, x + barWidth * 0.5f + 0.015f, y + 0.016f, 0.0045f);
}

// ============================================================================
// REQUIREMENT 4: GAMEPLAY TOP HUD (COLLECTIBLES & 3 PLAYER CARDS)
// ============================================================================

void drawHUD(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float mouseX,
    float mouseY
) {
    // Automatically deselect any card that became locked due to collectibles dropping
    if (selectedCard != -1 && !isCardUnlocked(selectedCard)) {
        selectedCard = -1;
    }

    // Top HUD kitchen shelf background bar
    glUniform3f(colorLocation, 0.16f, 0.18f, 0.22f);
    drawRectangle(transformLocation, 0.0f, 0.84f, 2.8f, 0.32f);

    // Bottom trim border of the HUD bar
    glUniform3f(colorLocation, 0.78f, 0.65f, 0.42f);
    drawRectangle(transformLocation, 0.0f, 0.68f, 2.8f, 0.016f);

    // --- 3 Player Cards on the Left ---
    const int costs[3] = { MELEE_COST, RANGED_COST, DEFENSE_COST };
    const char* cardLabels[3] = { "SWATTER", "SPRAY", "STICKY" };
    const float cardCentersX[3] = { -0.74f, -0.44f, -0.14f };
    const float cardCenterY = 0.84f;
    const float cardW = 0.25f;
    const float cardH = 0.26f;

    for (int i = 0; i < 3; i++) {
        bool unlocked = isCardUnlocked(i);
        bool hovered = false;

        // Strict rubric rule: Locked cards must NOT respond to mouse hover
        if (unlocked) {
            hovered = isPointInRect(mouseX, mouseY, cardCentersX[i], cardCenterY, cardW, cardH);
        }

        bool isSelected = (unlocked && selectedCard == i);

        // Card outer border
        if (!unlocked) {
            // Grayscale border when locked
            glUniform3f(colorLocation, 0.32f, 0.32f, 0.32f);
        } else if (isSelected) {
            // Bright green border when selected for placement
            glUniform3f(colorLocation, 0.25f, 1.0f, 0.40f);
        } else if (hovered) {
            // Bright yellow border when hovered
            glUniform3f(colorLocation, 1.0f, 0.92f, 0.25f);
        } else {
            // Normal unlocked card frame
            glUniform3f(colorLocation, 0.85f, 0.72f, 0.45f);
        }
        drawRectangle(transformLocation, cardCentersX[i], cardCenterY, cardW, cardH);

        // Card inner background
        if (!unlocked) {
            // Strict grayscale background when locked
            glUniform3f(colorLocation, 0.22f, 0.22f, 0.22f);
        } else if (hovered || isSelected) {
            glUniform3f(colorLocation, 0.28f, 0.36f, 0.46f);
        } else {
            glUniform3f(colorLocation, 0.20f, 0.26f, 0.34f);
        }
        drawRectangle(transformLocation, cardCentersX[i], cardCenterY, cardW - 0.02f, cardH - 0.02f);

        // Card name banner at top of card
        if (!unlocked) {
            glUniform3f(colorLocation, 0.55f, 0.55f, 0.55f);
        } else {
            glUniform3f(colorLocation, 1.0f, 1.0f, 1.0f);
        }
        drawCenteredText(transformLocation, cardLabels[i], cardCentersX[i], cardCenterY + 0.095f, 0.0042f);

        // Composite player icon in the middle of the card
        float iconY = cardCenterY + 0.005f;
        if (i == 0) {
            drawSwatterIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                            cardCentersX[i], iconY, 0.075f, !unlocked);
        } else if (i == 1) {
            drawSprayBottleIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                                cardCentersX[i], iconY, 0.075f, !unlocked);
        } else {
            drawStickySheetIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount,
                                cardCentersX[i], iconY, 0.072f, !unlocked);
        }

        // Bottom cost strip inside card
        if (!unlocked) {
            glUniform3f(colorLocation, 0.16f, 0.16f, 0.16f);
        } else {
            glUniform3f(colorLocation, 0.12f, 0.16f, 0.22f);
        }
        drawRectangle(transformLocation, cardCentersX[i], cardCenterY - 0.092f, cardW - 0.02f, 0.052f);

        // Small collectible icon + cost number on the card
        drawCollectibleIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount,
                            cardCentersX[i] - 0.055f, cardCenterY - 0.092f, 0.020f, !unlocked);

        if (!unlocked) {
            glUniform3f(colorLocation, 0.58f, 0.58f, 0.58f);
        } else {
            glUniform3f(colorLocation, 1.0f, 0.92f, 0.35f);
        }
        drawNumber(transformLocation, costs[i], cardCentersX[i] - 0.015f, cardCenterY - 0.074f, 0.0052f);
    }

    // --- Collectibles Counter Box on the Right ---
    float coinBoxX = 0.34f;
    float coinBoxY = 0.84f;
    float coinBoxW = 0.44f;
    float coinBoxH = 0.18f;

    glUniform3f(colorLocation, 0.85f, 0.72f, 0.35f);
    drawRectangle(transformLocation, coinBoxX, coinBoxY, coinBoxW, coinBoxH);

    glUniform3f(colorLocation, 0.12f, 0.15f, 0.20f);
    drawRectangle(transformLocation, coinBoxX, coinBoxY, coinBoxW - 0.02f, coinBoxH - 0.02f);

    // Composite soap-bubble coin icon
    drawCollectibleIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount,
                        coinBoxX - 0.13f, coinBoxY, 0.055f, false);

    // "COINS" label and current collectible value
    glUniform3f(colorLocation, 0.80f, 0.90f, 1.0f);
    drawText(transformLocation, "COINS", coinBoxX - 0.04f, coinBoxY + 0.062f, 0.0048f);

    glUniform3f(colorLocation, 1.0f, 0.92f, 0.28f);
    drawNumber(transformLocation, collectibles, coinBoxX - 0.04f, coinBoxY + 0.012f, 0.0085f);

    // --- Pause Button on Far Right of HUD ---
    float pauseBtnX = 0.78f;
    float pauseBtnY = 0.84f;
    float pauseBtnW = 0.26f;
    float pauseBtnH = 0.14f;
    bool pauseHovered = isPointInRect(mouseX, mouseY, pauseBtnX, pauseBtnY, pauseBtnW, pauseBtnH);
    drawButton(transformLocation, colorLocation, pauseBtnX, pauseBtnY, pauseBtnW, pauseBtnH,
               "PAUSE", 0.0055f, pauseHovered, 0.28f, 0.35f, 0.48f);
}

// ============================================================================
// PLACEHOLDER PLAYING SCENE (To test HUD, Cards, and Unit Health Bars)
// ============================================================================

void drawPlayingPlaceholderScene(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart
) {
    // Kitchen countertop background below HUD
    glUniform3f(colorLocation, 0.88f, 0.84f, 0.76f);
    drawRectangle(transformLocation, 0.0f, -0.16f, 2.8f, 1.68f);

    // Food Home Base zone on the left (placeholder for Member 2)
    glUniform3f(colorLocation, 0.75f, 0.28f, 0.22f);
    drawRectangle(transformLocation, -0.86f, -0.12f, 0.16f, 1.32f);
    glUniform3f(colorLocation, 1.0f, 0.90f, 0.70f);
    drawCenteredText(transformLocation, "HOME", -0.86f, -0.10f, 0.0042f);

    // 3 Horizontal Lanes (placeholder for Member 2)
    const float laneY[3] = { 0.28f, -0.12f, -0.52f };
    for (int lane = 0; lane < 3; lane++) {
        // Lane border
        glUniform3f(colorLocation, 0.62f, 0.56f, 0.48f);
        drawRectangle(transformLocation, 0.06f, laneY[lane], 1.64f, 0.34f);

        // Lane countertop surface
        if (lane % 2 == 0) {
            glUniform3f(colorLocation, 0.94f, 0.90f, 0.82f);
        } else {
            glUniform3f(colorLocation, 0.90f, 0.86f, 0.78f);
        }
        drawRectangle(transformLocation, 0.06f, laneY[lane], 1.60f, 0.30f);
    }

    // --- Placeholder Units + Reusable Health Bars (Requirement 5) ---

    // Lane 0: Melee Fly Swatter Player (5/5 HP) + Incoming Fly Enemy (3/5 HP)
    drawSwatterIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                    -0.45f, laneY[0] - 0.02f, 0.09f, false);
    drawHealthBar(transformLocation, colorLocation, -0.45f, laneY[0] + 0.12f, testMeleeHP, 5);

    drawPlaceholderInsect(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                          0.45f, laneY[0] - 0.02f, 0.11f);
    drawHealthBar(transformLocation, colorLocation, 0.45f, laneY[0] + 0.12f, testEnemyHP, 5);

    // Lane 1: Long Range Spray Bottle Player (4/5 HP)
    drawSprayBottleIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                        -0.25f, laneY[1] - 0.02f, 0.09f, false);
    drawHealthBar(transformLocation, colorLocation, -0.25f, laneY[1] + 0.12f, testRangedHP, 5);

    // Lane 2: Stationary Defense Sticky Sheet Player (7/7 HP)
    drawStickySheetIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount,
                        -0.55f, laneY[2] - 0.02f, 0.09f, false);
    drawHealthBar(transformLocation, colorLocation, -0.55f, laneY[2] + 0.12f, testDefenseHP, 7);

    // Bottom helper bar showing test controls
    glUniform3f(colorLocation, 0.14f, 0.16f, 0.20f);
    drawRectangle(transformLocation, 0.0f, -0.90f, 2.8f, 0.16f);

    glUniform3f(colorLocation, 0.85f, 0.90f, 0.95f);
    drawCenteredText(transformLocation, "UP DOWN COINS   H DAMAGE   ESC PAUSE   4 WIN   5 LOSE", 0.0f, -0.90f, 0.0042f);
}

// ============================================================================
// REQUIREMENT 1: MAIN MENU
// ============================================================================

void drawMainMenu(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float mouseX,
    float mouseY
) {
    // Kitchen wall background
    glUniform3f(colorLocation, 0.14f, 0.18f, 0.24f);
    drawRectangle(transformLocation, 0.0f, 0.0f, 2.8f, 2.0f);

    // Decorative kitchen countertop strip at the bottom
    glUniform3f(colorLocation, 0.78f, 0.68f, 0.54f);
    drawRectangle(transformLocation, 0.0f, -0.82f, 2.8f, 0.36f);
    glUniform3f(colorLocation, 0.58f, 0.48f, 0.36f);
    drawRectangle(transformLocation, 0.0f, -0.64f, 2.8f, 0.025f);

    // Center menu board frame
    glUniform3f(colorLocation, 0.85f, 0.72f, 0.45f);
    drawRectangle(transformLocation, 0.0f, 0.06f, 1.56f, 1.44f);
    glUniform3f(colorLocation, 0.18f, 0.24f, 0.32f);
    drawRectangle(transformLocation, 0.0f, 0.06f, 1.50f, 1.38f);

    // Title Banner
    glUniform3f(colorLocation, 0.12f, 0.16f, 0.22f);
    drawRectangle(transformLocation, 0.0f, 0.50f, 1.36f, 0.32f);

    glUniform3f(colorLocation, 1.0f, 0.90f, 0.28f);
    drawCenteredText(transformLocation, "KITCHEN DEFENSE", 0.0f, 0.55f, 0.011f);

    glUniform3f(colorLocation, 0.65f, 0.90f, 1.0f);
    drawCenteredText(transformLocation, "DETERGENTS VS INSECTS", 0.0f, 0.41f, 0.0062f);

    // Decorative composite characters flanking the buttons
    drawSwatterIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                    -0.52f, -0.08f, 0.16f, false);
    drawSprayBottleIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                        -0.52f, -0.45f, 0.13f, false);
    drawPlaceholderInsect(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                          0.52f, -0.18f, 0.18f);

    // START Button
    float startX = 0.0f, startY = 0.02f, btnW = 0.68f, btnH = 0.20f;
    bool startHovered = isPointInRect(mouseX, mouseY, startX, startY, btnW, btnH);
    drawButton(transformLocation, colorLocation, startX, startY, btnW, btnH,
               "START GAME", 0.0082f, startHovered, 0.18f, 0.62f, 0.30f);

    // EXIT Button
    float exitX = 0.0f, exitY = -0.28f;
    bool exitHovered = isPointInRect(mouseX, mouseY, exitX, exitY, btnW, btnH);
    drawButton(transformLocation, colorLocation, exitX, exitY, btnW, btnH,
               "EXIT GAME", 0.0082f, exitHovered, 0.72f, 0.22f, 0.22f);
}

// ============================================================================
// REQUIREMENT 2: PAUSE MENU
// ============================================================================

void drawPauseMenu(
    GLint transformLocation,
    GLint colorLocation,
    float mouseX,
    float mouseY
) {
    // Outer pause modal frame
    glUniform3f(colorLocation, 0.08f, 0.09f, 0.12f);
    drawRectangle(transformLocation, 0.0f, -0.05f, 1.16f, 1.24f);

    glUniform3f(colorLocation, 0.85f, 0.72f, 0.42f);
    drawRectangle(transformLocation, 0.0f, -0.05f, 1.12f, 1.20f);

    glUniform3f(colorLocation, 0.16f, 0.20f, 0.28f);
    drawRectangle(transformLocation, 0.0f, -0.05f, 1.06f, 1.14f);

    // Title
    glUniform3f(colorLocation, 1.0f, 0.92f, 0.30f);
    drawCenteredText(transformLocation, "GAME PAUSED", 0.0f, 0.36f, 0.010f);

    float btnW = 0.78f;
    float btnH = 0.17f;

    // 1. CONTINUE Button
    bool contHovered = isPointInRect(mouseX, mouseY, 0.0f, 0.10f, btnW, btnH);
    drawButton(transformLocation, colorLocation, 0.0f, 0.10f, btnW, btnH,
               "CONTINUE", 0.0075f, contHovered, 0.20f, 0.62f, 0.32f);

    // 2. RESTART Button
    bool restartHovered = isPointInRect(mouseX, mouseY, 0.0f, -0.14f, btnW, btnH);
    drawButton(transformLocation, colorLocation, 0.0f, -0.14f, btnW, btnH,
               "RESTART", 0.0075f, restartHovered, 0.25f, 0.48f, 0.75f);

    // 3. RETURN TO MAIN MENU Button
    bool menuHovered = isPointInRect(mouseX, mouseY, 0.0f, -0.38f, btnW, btnH);
    drawButton(transformLocation, colorLocation, 0.0f, -0.38f, btnW, btnH,
               "MAIN MENU", 0.0075f, menuHovered, 0.68f, 0.26f, 0.24f);
}

// ============================================================================
// REQUIREMENT 3: WIN / LOSE MENU
// ============================================================================

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
    // Full screen backdrop
    if (isWin) {
        glUniform3f(colorLocation, 0.10f, 0.24f, 0.16f);
    } else {
        glUniform3f(colorLocation, 0.24f, 0.10f, 0.12f);
    }
    drawRectangle(transformLocation, 0.0f, 0.0f, 2.8f, 2.0f);

    // Center modal frame
    if (isWin) {
        glUniform3f(colorLocation, 0.95f, 0.82f, 0.25f);
    } else {
        glUniform3f(colorLocation, 0.85f, 0.25f, 0.25f);
    }
    drawRectangle(transformLocation, 0.0f, 0.0f, 1.32f, 1.32f);

    glUniform3f(colorLocation, 0.14f, 0.18f, 0.24f);
    drawRectangle(transformLocation, 0.0f, 0.0f, 1.26f, 1.26f);

    // Header text and themed composite emblem
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

        drawPlaceholderInsect(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                              -0.48f, 0.42f, 0.12f);
        drawPlaceholderInsect(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                              0.48f, 0.42f, 0.12f);
    }

    float btnW = 0.78f;
    float btnH = 0.18f;

    // 1. RESTART Button
    bool restartHovered = isPointInRect(mouseX, mouseY, 0.0f, -0.04f, btnW, btnH);
    drawButton(transformLocation, colorLocation, 0.0f, -0.04f, btnW, btnH,
               "RESTART", 0.0080f, restartHovered, 0.20f, 0.60f, 0.32f);

    // 2. RETURN TO MAIN MENU Button
    bool menuHovered = isPointInRect(mouseX, mouseY, 0.0f, -0.32f, btnW, btnH);
    drawButton(transformLocation, colorLocation, 0.0f, -0.32f, btnW, btnH,
               "MAIN MENU", 0.0080f, menuHovered, 0.68f, 0.26f, 0.24f);
}

// ============================================================================
// MASTER UI RENDER & INPUT DISPATCH
// ============================================================================

void drawUI(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float mouseX,
    float mouseY
) {
    if (currentGameState == STATE_MAIN_MENU) {
        drawMainMenu(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart, mouseX, mouseY);
    }
    else if (currentGameState == STATE_CUTSCENE) {
        // Placeholder screen for Member 4's Cutscene state with button to enter gameplay
        glUniform3f(colorLocation, 0.12f, 0.15f, 0.20f);
        drawRectangle(transformLocation, 0.0f, 0.0f, 2.8f, 2.0f);

        glUniform3f(colorLocation, 1.0f, 0.90f, 0.30f);
        drawCenteredText(transformLocation, "INTRO CUTSCENE", 0.0f, 0.35f, 0.011f);

        drawSwatterIcon(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                        -0.35f, 0.0f, 0.16f, false);
        drawPlaceholderInsect(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                              0.35f, 0.0f, 0.18f);

        bool skipHovered = isPointInRect(mouseX, mouseY, 0.0f, -0.42f, 0.72f, 0.18f);
        drawButton(transformLocation, colorLocation, 0.0f, -0.42f, 0.72f, 0.18f,
                   "CONTINUE", 0.0075f, skipHovered, 0.20f, 0.60f, 0.32f);
    }
    else if (currentGameState == STATE_PLAYING) {
        drawPlayingPlaceholderScene(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart);
        drawHUD(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart, mouseX, mouseY);
    }
    else if (currentGameState == STATE_PAUSED) {
        // Draw the paused game + HUD behind the Pause Menu
        drawPlayingPlaceholderScene(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart);
        drawHUD(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart, -99.0f, -99.0f);
        drawPauseMenu(transformLocation, colorLocation, mouseX, mouseY);
    }
    else if (currentGameState == STATE_WON) {
        drawWinLoseMenu(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart, true, mouseX, mouseY);
    }
    else if (currentGameState == STATE_LOST) {
        drawWinLoseMenu(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart, false, mouseX, mouseY);
    }
}

void handleUIMouseClick(
    GLFWwindow* window,
    float mouseX,
    float mouseY
) {
    if (currentGameState == STATE_MAIN_MENU) {
        // START Button
        if (isPointInRect(mouseX, mouseY, 0.0f, 0.02f, 0.68f, 0.20f)) {
            resetGamePlaceholders();
            currentGameState = STATE_PLAYING;
            return;
        }
        // EXIT Button
        if (isPointInRect(mouseX, mouseY, 0.0f, -0.28f, 0.68f, 0.20f)) {
            glfwSetWindowShouldClose(window, true);
            return;
        }
    }
    else if (currentGameState == STATE_CUTSCENE) {
        if (isPointInRect(mouseX, mouseY, 0.0f, -0.42f, 0.72f, 0.18f)) {
            currentGameState = STATE_PLAYING;
            return;
        }
    }
    else if (currentGameState == STATE_PLAYING) {
        // Check 3 Player Cards in Top HUD
        const float cardCentersX[3] = { -0.74f, -0.44f, -0.14f };
        const float cardCenterY = 0.84f;
        const float cardW = 0.25f;
        const float cardH = 0.26f;

        for (int i = 0; i < 3; i++) {
            if (isPointInRect(mouseX, mouseY, cardCentersX[i], cardCenterY, cardW, cardH)) {
                // Strict rubric rule: Locked cards must NOT respond to clicks
                if (isCardUnlocked(i)) {
                    if (selectedCard == i) {
                        selectedCard = -1;
                    } else {
                        selectedCard = i;
                    }
                }
                return;
            }
        }

        // Check Pause Button on Top Right of HUD
        if (isPointInRect(mouseX, mouseY, 0.78f, 0.84f, 0.26f, 0.14f)) {
            currentGameState = STATE_PAUSED;
            return;
        }

        // If an unlocked card is selected and player clicks a lane, simulate spending coins
        if (selectedCard != -1 && isCardUnlocked(selectedCard) && mouseY < 0.65f && mouseY > -0.75f) {
            const int costs[3] = { MELEE_COST, RANGED_COST, DEFENSE_COST };
            collectibles -= costs[selectedCard];
            if (collectibles < 0) {
                collectibles = 0;
            }
            if (!isCardUnlocked(selectedCard)) {
                selectedCard = -1;
            }
            return;
        }
    }
    else if (currentGameState == STATE_PAUSED) {
        float btnW = 0.78f;
        float btnH = 0.17f;

        // CONTINUE
        if (isPointInRect(mouseX, mouseY, 0.0f, 0.10f, btnW, btnH)) {
            currentGameState = STATE_PLAYING;
            return;
        }
        // RESTART
        if (isPointInRect(mouseX, mouseY, 0.0f, -0.14f, btnW, btnH)) {
            resetGamePlaceholders();
            currentGameState = STATE_PLAYING;
            return;
        }
        // RETURN TO MAIN MENU
        if (isPointInRect(mouseX, mouseY, 0.0f, -0.38f, btnW, btnH)) {
            resetGamePlaceholders();
            currentGameState = STATE_MAIN_MENU;
            return;
        }
    }
    else if (currentGameState == STATE_WON || currentGameState == STATE_LOST) {
        float btnW = 0.78f;
        float btnH = 0.18f;

        // RESTART
        if (isPointInRect(mouseX, mouseY, 0.0f, -0.04f, btnW, btnH)) {
            resetGamePlaceholders();
            currentGameState = STATE_PLAYING;
            return;
        }
        // RETURN TO MAIN MENU
        if (isPointInRect(mouseX, mouseY, 0.0f, -0.32f, btnW, btnH)) {
            resetGamePlaceholders();
            currentGameState = STATE_MAIN_MENU;
            return;
        }
    }
}

void handleUIKeyPress(
    GLFWwindow* window,
    int key,
    int action
) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) {
        return;
    }

    // ESC toggles Pause Menu during gameplay
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        if (currentGameState == STATE_PLAYING) {
            currentGameState = STATE_PAUSED;
        } else if (currentGameState == STATE_PAUSED) {
            currentGameState = STATE_PLAYING;
        }
        return;
    }

    // Standalone testing keys for switching states (1-5)
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_1) currentGameState = STATE_MAIN_MENU;
        if (key == GLFW_KEY_2) currentGameState = STATE_PLAYING;
        if (key == GLFW_KEY_3) currentGameState = STATE_PAUSED;
        if (key == GLFW_KEY_4) currentGameState = STATE_WON;
        if (key == GLFW_KEY_5) currentGameState = STATE_LOST;
        if (key == GLFW_KEY_C) currentGameState = STATE_CUTSCENE;
    }

    // Test keys for adding/subtracting placeholder collectibles (+ / UP and - / DOWN)
    if (key == GLFW_KEY_UP || key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) {
        collectibles += 5;
    }
    else if (key == GLFW_KEY_DOWN || key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) {
        collectibles -= 5;
        if (collectibles < 0) {
            collectibles = 0;
        }
        if (selectedCard != -1 && !isCardUnlocked(selectedCard)) {
            selectedCard = -1;
        }
    }

    // Test key 'H' to damage placeholder units and see health bars decrease
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        testMeleeHP   = (testMeleeHP > 0)   ? testMeleeHP - 1   : 5;
        testRangedHP  = (testRangedHP > 0)  ? testRangedHP - 1  : 5;
        testDefenseHP = (testDefenseHP > 0) ? testDefenseHP - 1 : 7;
        testEnemyHP   = (testEnemyHP > 0)   ? testEnemyHP - 1   : 5;
    }

    // Test key 'R' to reset placeholder health and collectibles
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        resetGamePlaceholders();
    }
}

