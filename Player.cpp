#include "Player.h"
#include "PlayerCombat.h"
#include "Environment.h"
#include "Game.h"
#include "Renderer.h"
#include "UI.h"
#include <cmath>

// player movement and jump/duck settings
const float PLAYER_SPEED       = 0.14f;
const float JUMP_DUCK_DURATION = 0.65f;
const float JUMP_OFFSET_Y      = 0.065f;
const float DUCK_OFFSET_Y      = -0.045f;

// active players on the lanes
static std::vector<Player> players;

std::vector<Player>& getPlayers()
{
    return players;
}

// set color mixed with red based on how much health the player lost
void setPlayerColor(GLint colorLocation, float r, float g, float b, int hp, int maxHp)
{
    float lostRatio = 0.0f;
    if (maxHp > 0) {
        lostRatio = (float)(maxHp - hp) / (float)maxHp;
    }
    if (lostRatio < 0.0f) lostRatio = 0.0f;
    if (lostRatio > 1.0f) lostRatio = 1.0f;

    // increase red tint as health decreases
    float redAmount = lostRatio * 0.75f;
    float finalR = r + (1.0f - r) * redAmount;
    float finalG = g * (1.0f - redAmount);
    float finalB = b * (1.0f - redAmount);

    glUniform3f(colorLocation, finalR, finalG, finalB);
}

int spawnPlayer(int type, int lane)
{
    Environment& env = getEnvironment();
    if (type < PLAYER_MELEE || type > PLAYER_DEFENSE || lane < 0 || lane >= env.getLaneCount()) {
        return -1;
    }

    Player p;
    p.type = type;
    p.lane = lane;
    p.x = env.getLaneStartX() + 0.08f;
    p.y = env.getLaneCenterY(lane) - 0.015f;

    if (type == PLAYER_DEFENSE) {
        p.maxHp = DEFENSE_MAX_HP; // 7 hits
    } else if (type == PLAYER_RANGED) {
        p.maxHp = RANGED_MAX_HP;  // 5 lives
    } else {
        p.maxHp = MELEE_MAX_HP;   // 5 lives
    }

    p.hp = p.maxHp;
    p.pose = POSE_NORMAL;
    p.poseTimer = 0.0f;
    p.blockedByEnemy = false;
    p.animTime = 0.0f;
    p.attackCooldown = 0.0f;
    p.attackAnimation = 0.0f;

    players.push_back(p);
    return (int)players.size() - 1;
}

void resetPlayers()
{
    players.clear();
}

void damagePlayer(int index, int damage)
{
    if (index < 0 || index >= (int)players.size() || damage <= 0) {
        return;
    }

    players[index].hp -= damage;

    // remove player from scene when defeated
    if (players[index].hp <= 0) {
        players.erase(players.begin() + index);
    }
}

void setPlayerBlocked(int index, bool blocked)
{
    if (index < 0 || index >= (int)players.size()) {
        return;
    }
    players[index].blockedByEnemy = blocked;
}

void clearAllPlayerBlocks()
{
    for (Player& player : players) {
        player.blockedByEnemy = false;
    }
}

void updatePlayers(float dt)
{
    if (dt <= 0.0f || !std::isfinite(dt)) return;

    Environment& env = getEnvironment();
    float maxRightX = env.getLaneEndX() - 0.06f;

    for (Player& player : players) {
        player.animTime += dt;

        // update jump or duck timer
        if (player.pose != POSE_NORMAL) {
            player.poseTimer -= dt;
            if (player.poseTimer <= 0.0f) {
                player.pose = POSE_NORMAL;
                player.poseTimer = 0.0f;
            }
        }

        // keep player Y inside its lane plus jump/duck offset
        float baseY = env.getLaneCenterY(player.lane) - 0.015f;
        if (player.pose == POSE_JUMPING) {
            player.y = baseY + JUMP_OFFSET_Y;
        } else if (player.pose == POSE_DUCKING) {
            player.y = baseY + DUCK_OFFSET_Y;
        } else {
            player.y = baseY;
        }

        // melee and ranged move right automatically; defense stays stationary
        if (player.type != PLAYER_DEFENSE && !player.blockedByEnemy) {
            player.x += PLAYER_SPEED * dt;
            if (player.x > maxRightX) {
                player.x = maxRightX;
            }
        }
    }
}

void handlePlayerKeyPress(int key, int action)
{
    if (!isGameplayRunning()) return;
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    int newPose = POSE_NORMAL;
    if (key == GLFW_KEY_W || key == GLFW_KEY_UP) {
        newPose = POSE_JUMPING;
    } else if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN) {
        newPose = POSE_DUCKING;
    } else {
        return;
    }

    Environment& env = getEnvironment();
    for (Player& player : players) {
        // defense players are stationary and ignore jump and duck
        if (player.type == PLAYER_DEFENSE) {
            continue;
        }

        // apply jump or duck to every eligible player in all selected lanes
        if (env.isLaneSelected(player.lane)) {
            player.pose = newPose;
            player.poseTimer = JUMP_DUCK_DURATION;
        }
    }
}

// detailed melee fly swatter player model
void drawMeleePlayer(
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
    setPlayerColor(colorLocation, 0.18f, 0.20f, 0.24f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x - 0.22f * scaleX + step, p.y - 0.98f * scaleY, 0.30f * scaleX, 0.18f * scaleY);
    drawRectangle(transformLocation, p.x + 0.22f * scaleX - step, p.y - 0.98f * scaleY, 0.30f * scaleX, 0.18f * scaleY);

    // handle grip
    setPlayerColor(colorLocation, 0.25f, 0.25f, 0.30f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x, p.y - 0.50f * scaleY, 0.18f * scaleX, 0.75f * scaleY);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x, p.y - 0.85f * scaleY, 0.10f * scaleX);

    // neck connector
    setPlayerColor(colorLocation, 0.85f, 0.25f, 0.30f, p.hp, p.maxHp);
    drawTriangle(transformLocation, triangleVertexStart, p.x, p.y - 0.08f * scaleY, 0.52f * scaleX, 0.35f * scaleY, 180.0f);

    // swatter head outer frame
    setPlayerColor(colorLocation, 0.95f, 0.28f, 0.32f, p.hp, p.maxHp);
    drawRectangle(transformLocation, headX, headY + 0.38f * scaleY, 0.96f * scaleX, 0.86f * scaleY);

    // inner mesh background
    setPlayerColor(colorLocation, 0.55f, 0.12f, 0.16f, p.hp, p.maxHp);
    drawRectangle(transformLocation, headX, headY + 0.38f * scaleY, 0.76f * scaleX, 0.66f * scaleY);

    // mesh grid lines
    setPlayerColor(colorLocation, 1.0f, 0.55f, 0.58f, p.hp, p.maxHp);
    drawRectangle(transformLocation, headX - 0.20f * scaleX, headY + 0.38f * scaleY, 0.06f * scaleX, 0.66f * scaleY);
    drawRectangle(transformLocation, headX,                  headY + 0.38f * scaleY, 0.06f * scaleX, 0.66f * scaleY);
    drawRectangle(transformLocation, headX + 0.20f * scaleX, headY + 0.38f * scaleY, 0.06f * scaleX, 0.66f * scaleY);
    drawRectangle(transformLocation, headX, headY + 0.18f * scaleY, 0.76f * scaleX, 0.06f * scaleY);
    drawRectangle(transformLocation, headX, headY + 0.38f * scaleY, 0.76f * scaleX, 0.06f * scaleY);
    drawRectangle(transformLocation, headX, headY + 0.58f * scaleY, 0.76f * scaleX, 0.06f * scaleY);

    // eyes looking right
    setPlayerColor(colorLocation, 0.98f, 0.98f, 0.95f, p.hp, p.maxHp);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, headX - 0.14f * scaleX, headY + 0.44f * scaleY, 0.13f * scaleX);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, headX + 0.18f * scaleX, headY + 0.44f * scaleY, 0.13f * scaleX);

    setPlayerColor(colorLocation, 0.10f, 0.14f, 0.18f, p.hp, p.maxHp);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, headX - 0.10f * scaleX, headY + 0.44f * scaleY, 0.06f * scaleX);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, headX + 0.22f * scaleX, headY + 0.44f * scaleY, 0.06f * scaleX);

    // swinging weapon arm in front
    setPlayerColor(colorLocation, 0.92f, 0.30f, 0.32f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x + 0.48f * scaleX, p.y - 0.18f * scaleY, 0.55f * scaleX, 0.14f * scaleY, armSwing);
}

// detailed long range spray bottle player model
void drawRangedPlayer(
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
    setPlayerColor(colorLocation, 0.14f, 0.35f, 0.55f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x - 0.22f * scaleX + step, p.y - 0.90f * scaleY, 0.30f * scaleX, 0.16f * scaleY);
    drawRectangle(transformLocation, p.x + 0.18f * scaleX - step, p.y - 0.90f * scaleY, 0.30f * scaleX, 0.16f * scaleY);

    // bottle body and shoulder taper
    setPlayerColor(colorLocation, 0.18f, 0.68f, 0.95f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x - 0.05f * scaleX, p.y - 0.38f * scaleY, 0.78f * scaleX, 0.88f * scaleY);
    drawTriangle(transformLocation, triangleVertexStart, p.x - 0.05f * scaleX, p.y + 0.22f * scaleY, 0.78f * scaleX, 0.38f * scaleY);

    // bottle label and stripe
    setPlayerColor(colorLocation, 0.95f, 0.98f, 1.0f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x - 0.05f * scaleX, p.y - 0.42f * scaleY, 0.56f * scaleX, 0.38f * scaleY);

    setPlayerColor(colorLocation, 0.10f, 0.45f, 0.80f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x - 0.05f * scaleX, p.y - 0.42f * scaleY, 0.42f * scaleX, 0.12f * scaleY);

    // eyes on bottle
    setPlayerColor(colorLocation, 1.0f, 1.0f, 1.0f, p.hp, p.maxHp);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x - 0.16f * scaleX, p.y - 0.06f * scaleY, 0.11f * scaleX);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x + 0.10f * scaleX, p.y - 0.06f * scaleY, 0.11f * scaleX);

    setPlayerColor(colorLocation, 0.10f, 0.15f, 0.22f, p.hp, p.maxHp);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x - 0.12f * scaleX, p.y - 0.06f * scaleY, 0.05f * scaleX);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x + 0.14f * scaleX, p.y - 0.06f * scaleY, 0.05f * scaleX);

    // yellow neck collar
    setPlayerColor(colorLocation, 0.95f, 0.85f, 0.20f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x - 0.05f * scaleX, p.y + 0.42f * scaleY, 0.32f * scaleX, 0.16f * scaleY);

    // spray head and nozzle
    setPlayerColor(colorLocation, 0.95f, 0.30f, 0.30f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x + 0.04f * scaleX, p.y + 0.62f * scaleY, 0.64f * scaleX, 0.24f * scaleY);

    // animated trigger lever
    setPlayerColor(colorLocation, 0.88f, 0.18f, 0.18f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x + 0.24f * scaleX, p.y + 0.38f * scaleY, 0.10f * scaleX, 0.32f * scaleY, 22.0f + triggerPull);

    // Mist appears only while the trigger is firing; moving shots are separate.
    if (p.attackAnimation > 0.0f) {
        setPlayerColor(colorLocation, 0.55f, 0.92f, 1.0f, p.hp, p.maxHp);
        drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x + (0.50f + sprayPulse * 0.12f) * scaleX, p.y + 0.66f * scaleY, 0.07f * scaleX);
        drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x + (0.64f + sprayPulse * 0.12f) * scaleX, p.y + 0.56f * scaleY, 0.05f * scaleX);
    }
}

// detailed stationary defense sticky sheet player model
void drawDefensePlayer(
    const Player& p,
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount
) {
    float scale = 0.085f;

    // sturdy bottom stand
    setPlayerColor(colorLocation, 0.45f, 0.28f, 0.12f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x, p.y - 0.78f * scale, 1.08f * scale, 0.18f * scale);

    // outer cardboard frame
    setPlayerColor(colorLocation, 0.65f, 0.42f, 0.18f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x, p.y - 0.05f * scale, 0.88f * scale, 1.35f * scale);

    // top hanging roller
    setPlayerColor(colorLocation, 0.82f, 0.58f, 0.28f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x, p.y + 0.68f * scale, 0.98f * scale, 0.20f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x - 0.49f * scale, p.y + 0.68f * scale, 0.10f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x + 0.49f * scale, p.y + 0.68f * scale, 0.10f * scale);

    // yellow sticky glue surface
    setPlayerColor(colorLocation, 0.98f, 0.88f, 0.22f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x, p.y - 0.08f * scale, 0.68f * scale, 1.08f * scale);

    // glue drops
    setPlayerColor(colorLocation, 0.92f, 0.65f, 0.10f, p.hp, p.maxHp);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x - 0.15f * scale, p.y + 0.20f * scale, 0.12f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x + 0.16f * scale, p.y - 0.05f * scale, 0.14f * scale);
    drawCircle(transformLocation, circleVertexStart, circleVertexCount, p.x - 0.10f * scale, p.y - 0.35f * scale, 0.11f * scale);

    // green defense shield cross
    setPlayerColor(colorLocation, 0.35f, 0.80f, 0.30f, p.hp, p.maxHp);
    drawRectangle(transformLocation, p.x, p.y - 0.08f * scale, 0.42f * scale, 0.12f * scale);
    drawRectangle(transformLocation, p.x, p.y - 0.08f * scale, 0.12f * scale, 0.42f * scale);
}

void drawPlayers(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart
) {
    for (int i = 0; i < (int)players.size(); i++) {
        const Player& p = players[i];

        // A small green underline marks allies currently protected by a sheet.
        if (p.type != PLAYER_DEFENSE && getPlayerDamageReceiver(i, getEnvironment().getLaneEndX()) != i) {
            glUniform3f(colorLocation, 0.30f, 0.85f, 0.45f);
            drawRectangle(transformLocation, p.x, p.y - 0.105f, 0.09f, 0.008f);
        }

        if (p.type == PLAYER_MELEE) {
            drawMeleePlayer(p, transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart);
        } else if (p.type == PLAYER_RANGED) {
            drawRangedPlayer(p, transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart);
        } else {
            drawDefensePlayer(p, transformLocation, colorLocation, circleVertexStart, circleVertexCount);
        }

        // draw health bar above the player
        drawHealthBar(transformLocation, colorLocation, p.x, p.y + 0.105f, p.hp, p.maxHp);
    }
}

