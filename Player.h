#pragma once
#include <glad/gl.h>
#include <vector>

// player types
const int PLAYER_MELEE   = 0; // fly swatter
const int PLAYER_RANGED  = 1; // spray bottle
const int PLAYER_DEFENSE = 2; // sticky sheet

// starting health for each player type
const int MELEE_MAX_HP   = 5;
const int RANGED_MAX_HP  = 5;
const int DEFENSE_MAX_HP = 7;

// player poses for jumping and ducking
const int POSE_NORMAL  = 0;
const int POSE_JUMPING = 1;
const int POSE_DUCKING = 2;

// shared player hitbox half-size for collisions (Members 2, 3, 4)
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

// create a player at the left start of a lane
int spawnPlayer(int type, int lane);

// move players and update jump/duck timers
void updatePlayers(float dt);

// handle W/Up (jump) and S/Down (duck) in selected lanes
void handlePlayerKeyPress(int key, int action);

// reduce player health and remove if defeated
void damagePlayer(int index, int damage);

// stop or resume player movement during enemy collisions (for Member 3)
void setPlayerBlocked(int index, bool blocked);
void clearAllPlayerBlocks();

// get reference to active players list (for Members 2, 3, 4)
std::vector<Player>& getPlayers();

// reset players when starting or restarting the game
void resetPlayers();

// draw all players and their health bars
void drawPlayers(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart
);
