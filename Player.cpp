#include "Audio.h"
#include "Player.h"
#include "PlayerCombat.h"
#include "Environment.h"
#include "Game.h"
#include "Renderer.h"
#include "UI.h"
#include <cmath>
#include <algorithm>

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

    playSound(players[index].type == PLAYER_DEFENSE ? SoundId::SheetHit : SoundId::UnitDamage);
    players[index].hp -= damage;

    // remove player from scene when defeated
    if (players[index].hp <= 0) {
        playSound(SoundId::UnitDefeated);
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
        }
    }

    // Remove finished characters from rendering, controls and combat.
    for (int i = (int)players.size() - 1; i >= 0; --i) {
        if (players[i].type != PLAYER_DEFENSE &&
            players[i].x >= maxRightX) {
            players.erase(players.begin() + i);
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

    bool changed = false;
    Environment& env = getEnvironment();
    for (Player& player : players) {
        // defense players are stationary and ignore jump and duck
        if (player.type == PLAYER_DEFENSE) {
            continue;
        }

        // apply jump or duck to every eligible player in all selected lanes
        if (env.isLaneSelected(player.lane)) {
            changed = changed || player.pose != newPose;
            player.pose = newPose;
            player.poseTimer = JUMP_DUCK_DURATION;
        }
    }
    if (changed) playSound(newPose == POSE_JUMPING ? SoundId::Jump : SoundId::Duck);
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
            drawPictureTinted(transformLocation,"ui-fill",p.x,p.y-.105f,.09f,.008f);
        }

        int pose=p.pose==POSE_DUCKING ? POSE_DUCKING : POSE_NORMAL;
        int frame=p.blockedByEnemy ? 0 : int(p.animTime*8*2/3.14159265f)%4;
        if(p.attackAnimation>0) frame=4+std::min(3,int((1-p.attackAnimation/PLAYER_ATTACK_ANIMATION_TIME)*4));
        if(p.type==PLAYER_DEFENSE) {pose=POSE_NORMAL;frame=0;}
        float damage=1.0f-float(p.hp)/p.maxHp;
        drawCharacterImage(transformLocation,colorLocation,
            "player-"+std::to_string(p.type)+"-"+std::to_string(pose)+"-"+std::to_string(frame),p.x,p.y,damage);

        // draw health bar above the player
        drawHealthBar(transformLocation, colorLocation, p.x, p.y + 0.105f, p.hp, p.maxHp);
    }
}

