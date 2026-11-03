#pragma once
#include <glad/gl.h>
#include <vector>
#include "PlayerCombat.h"

//enemy types
const int ENEMY_EASY = 0; //fly with gun
const int ENEMY_MEDIUM = 1; //ant with slingshot
const int ENEMY_HARD = 2; //cockroach with claymore
const float ENEMY_HALF_WIDTH = 0.055f;

//shared enemy data
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

//enemy projectile data
struct EnemyProjectile{
    int type;
    int lane;

    float x;
    float y;
};

//create an enemy at the right end of a lane
int spawnEnemy(int type, int lane);

//move enemies and update their behavior
void updateEnemies(float dt);

//access the active enemies
std::vector<Enemy>& getEnemies();

//clear enemies when resetting the game
void resetEnemies();
void damageEnemy(int enemyId, int amount);

//draw enemy models
void drawEnemies(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart
);

//enemy information for player attacks
int countEnemies();
bool readEnemyTarget(int index, PlayerAttackTarget& target);

