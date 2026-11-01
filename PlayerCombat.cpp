#include "PlayerCombat.h"
#include "Environment.h"
#include "Game.h"
#include "Player.h"
#include "Renderer.h"
#include <cmath>

static PlayerEnemyAccess enemies;
static std::vector<PlayerProjectile> projectiles;

void connectPlayerCombatToEnemies(PlayerEnemyAccess access) { enemies = access; }
const std::vector<PlayerProjectile>& getPlayerProjectiles() { return projectiles; }

static bool enemiesConnected()
{
    return enemies.getCount && enemies.getTarget && enemies.damage;
}

// Horizontal swept collision: a fast droplet cannot skip a narrow enemy between
// frames. All attacks travel right and only inspect enemies in their own lane.
static int findEnemyInPath(int lane, float left, float right)
{
    if (!enemiesConnected()) return -1;
    int closestId = -1;
    float closestX = right + 1.0f;
    for (int i = 0; i < enemies.getCount(); i++) {
        PlayerAttackTarget target;
        if (!enemies.getTarget(i, target) || target.lane != lane) continue;
        if (target.x + target.halfWidth < left || target.x - target.halfWidth > right) continue;
        if (target.x < closestX) {
            closestX = target.x;
            closestId = target.id;
        }
    }
    return closestId;
}

static void advanceProjectiles(float dt)
{
    for (int i = (int)projectiles.size() - 1; i >= 0; i--) {
        PlayerProjectile& shot = projectiles[i];
        float oldX = shot.x;
        shot.x += SPRAY_PROJECTILE_SPEED * dt;
        float right = shot.x + SPRAY_PROJECTILE_RADIUS;
        if (right > getEnvironment().getLaneEndX()) right = getEnvironment().getLaneEndX();
        int enemyId = findEnemyInPath(shot.lane, oldX - SPRAY_PROJECTILE_RADIUS, right);
        if (enemyId >= 0) {
            enemies.damage(enemyId, 1);
            projectiles.erase(projectiles.begin() + i);
        } else if (shot.x - SPRAY_PROJECTILE_RADIUS > getEnvironment().getLaneEndX()) {
            projectiles.erase(projectiles.begin() + i);
        }
    }
}

void updatePlayerCombat(float dt)
{
    if (!isGameplayRunning() || dt <= 0.0f || !std::isfinite(dt)) return;

    // Short simulation steps keep firing and swept impacts consistent during a
    // slow frame. Like Lab 4, all movement/timers use elapsed seconds.
    while (dt > 0.0f) {
        float step = dt > 1.0f / 120.0f ? 1.0f / 120.0f : dt;
        std::vector<Player>& players = getPlayers();
        for (int i = 0; i < (int)players.size(); i++) {
            Player& p = players[i];
            p.attackAnimation -= step;
            if (p.attackAnimation < 0.0f) p.attackAnimation = 0.0f;
            if (p.type == PLAYER_DEFENSE || p.hp <= 0) continue;
            p.attackCooldown -= step;
            if (p.attackCooldown > 0.0f) continue;

            p.attackAnimation = PLAYER_ATTACK_ANIMATION_TIME;
            if (p.type == PLAYER_MELEE) {
                p.attackCooldown += MELEE_ATTACK_INTERVAL;
                int enemyId = findEnemyInPath(p.lane, p.x,
                    p.x + PLAYER_HALF_WIDTH + MELEE_ATTACK_REACH);
                if (enemyId >= 0) enemies.damage(enemyId, 1);
            } else if (p.type == PLAYER_RANGED) {
                p.attackCooldown += SPRAY_ATTACK_INTERVAL;
                // Begin just inside the front hitbox so a touching enemy is hit.
                projectiles.push_back({p.lane, p.x + PLAYER_HALF_WIDTH * 0.8f,
                                       p.y + 0.048f});
            }
        }
        advanceProjectiles(step);
        dt -= step;
    }
}

int getPlayerDamageReceiver(int playerIndex, float attackerX)
{
    const std::vector<Player>& players = getPlayers();
    if (playerIndex < 0 || playerIndex >= (int)players.size()) return -1;
    const Player& ally = players[playerIndex];
    if (attackerX < ally.x) return playerIndex;
    int receiver = playerIndex;
    float frontX = ally.x;
    for (int i = 0; i < (int)players.size(); i++) {
        const Player& sheet = players[i];
        if (sheet.type == PLAYER_DEFENSE && sheet.hp > 0 && sheet.lane == ally.lane &&
            sheet.x >= frontX && sheet.x <= attackerX) {
            receiver = i;
            frontX = sheet.x;
        }
    }
    return receiver;
}

void damagePlayerFromEnemy(int playerIndex, int damage, float attackerX)
{
    if (!isGameplayRunning()) return;
    int receiver = getPlayerDamageReceiver(playerIndex, attackerX);
    if (receiver >= 0) damagePlayer(receiver, damage);
}

void resetPlayerCombat()
{
    projectiles.clear();
    std::vector<Player>& players = getPlayers();
    for (int i = 0; i < (int)players.size(); i++) {
        players[i].attackCooldown = 0.0f;
        players[i].attackAnimation = 0.0f;
    }
}

void drawPlayerCombat(GLint t, GLint c, int firstCircle, int circleCount)
{
    for (int i = 0; i < (int)projectiles.size(); i++) {
        const PlayerProjectile& shot = projectiles[i];
        glUniform3f(c, 0.12f, 0.55f, 0.75f);
        drawRectangle(t, shot.x - 0.018f, shot.y, 0.045f, 0.008f);
        glUniform3f(c, 0.55f, 0.94f, 1.0f);
        drawCircle(t, firstCircle, circleCount, shot.x, shot.y, SPRAY_PROJECTILE_RADIUS);
        glUniform3f(c, 0.94f, 1.0f, 1.0f);
        drawCircle(t, firstCircle, circleCount, shot.x + 0.003f, shot.y + 0.004f, 0.004f);
    }
}
