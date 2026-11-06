#pragma once
#include <glad/gl.h>
#include <vector>
#include "PlayerCombat.h"

const int ENEMY_EASY = 0;   // fly with gun
const int ENEMY_MEDIUM = 1; // ant with slingshot
const int ENEMY_HARD = 2;   // cockroach with claymore
const float ENEMY_HALF_WIDTH = 0.055f;

struct Enemy{
    int id;
    int type; 
    int lane;

    float x;
    float y;

    int hp;
    int maxHp;

    bool blockedByPlayer;
    float attackCooldown;
    float attackAnimation;
};

struct EnemyProjectile{
    int type;
    int lane;

    float x;
    float y;
};

// Returns a stable enemy ID, or -1 for an invalid type or lane.
int spawnEnemy(int type, int lane);

void updateEnemies(float dt);

std::vector<Enemy>& getEnemies();

void resetEnemies();
void resizeEnemyPositions(float scale, float offset);
void damageEnemy(int enemyId, int amount);

void drawEnemies(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart
);

// readEnemyTarget accepts a vector index; damageEnemy accepts a stable ID.
int countEnemies();
bool readEnemyTarget(int index, PlayerAttackTarget& target);

