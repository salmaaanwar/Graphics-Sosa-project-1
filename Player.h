#pragma once
#include <glad/gl.h>
#include <vector>

const int PLAYER_MELEE   = 0; // fly swatter
const int PLAYER_RANGED  = 1; // spray bottle
const int PLAYER_DEFENSE = 2; // sticky sheet

const int MELEE_MAX_HP   = 5;
const int RANGED_MAX_HP  = 5;
const int DEFENSE_MAX_HP = 7;

const int POSE_NORMAL  = 0;
const int POSE_JUMPING = 1;
const int POSE_DUCKING = 2;

// Collision bounds shared by Members 2, 3 and 4, measured from the player's center.
const float PLAYER_HALF_WIDTH  = 0.045f;
const float PLAYER_HALF_HEIGHT = 0.075f;

struct Player {
    int type;
    int lane;
    float x;
    float y;
    int hp;
    int maxHp;
    int pose;
    float poseTimer;
    bool blockedByEnemy;
    float animTime;
    float attackCooldown;
    float attackAnimation;
};

// Returns the new index, or -1 for an invalid type or lane.
int spawnPlayer(int type, int lane);

void updatePlayers(float dt);

// handle W/Up (jump) and S/Down (duck) in selected lanes
void handlePlayerKeyPress(int key, int action);

void damagePlayer(int index, int damage);

// Member 3's collision update stops moving units while they touch an enemy.
void setPlayerBlocked(int index, bool blocked);
void clearAllPlayerBlocks();

// Active players shared by Members 2, 3 and 4.
std::vector<Player>& getPlayers();

void resetPlayers();

void drawPlayers(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart
);
