#pragma once
#include <glad/gl.h>
#include <vector>

const float MELEE_ATTACK_INTERVAL = 0.75f;
const float SPRAY_ATTACK_INTERVAL = 1.0f;
const float PLAYER_ATTACK_ANIMATION_TIME = 0.24f;
const float MELEE_ATTACK_REACH = 0.035f;
const float SPRAY_PROJECTILE_SPEED = 0.85f;
const float SPRAY_PROJECTILE_RADIUS = 0.012f;

// Read-only view of an enemy owned by Members 3/4. IDs are nonnegative and stable when
// another enemy is erased. The provider skips dead enemies; it owns all HP.
struct PlayerAttackTarget
{
    int id;
    int lane;
    float x;
    float halfWidth;
};

// Register once when the enemy module is ready. Re-read live targets for every
// attack, since damage may erase an enemy. No enemy health is copied here.
struct PlayerEnemyAccess
{
    int (*getCount)() = nullptr;
    bool (*getTarget)(int index, PlayerAttackTarget& target) = nullptr;
    void (*damage)(int enemyId, int amount) = nullptr;
};
void connectPlayerCombatToEnemies(PlayerEnemyAccess access);

struct PlayerProjectile
{
    int lane;
    float x;
    float y;
};
const std::vector<PlayerProjectile>& getPlayerProjectiles();
void updatePlayerCombat(float dt);
void resetPlayerCombat();
void resizePlayerProjectiles(float scale, float offset);
void drawPlayerCombat(GLint transformLocation, GLint colorLocation);

// Enemy melee/projectiles resolve this immediately before applying a hit. A
// same-lane sheet between the ally and the attacker absorbs the entire attack.
// Check the returned player's jump/duck pose in the enemy module, then call
// damagePlayer(receiver, damage). Indices must not be cached across removals.
int getPlayerDamageReceiver(int playerIndex, float attackerX);
void damagePlayerFromEnemy(int playerIndex, int damage, float attackerX);
