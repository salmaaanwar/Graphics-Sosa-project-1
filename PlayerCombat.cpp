#include "Audio.h"
#include "PlayerCombat.h"
#include "Environment.h"
#include "Game.h"
#include "Player.h"
#include "Renderer.h"
#include <cmath>

static const float MAX_COMBAT_STEP = 1.0f / 120.0f;

static PlayerEnemyAccess enemies;
static std::vector<PlayerProjectile> projectiles;

void resizePlayerProjectiles(float scale, float offset)
{
    for (PlayerProjectile& shot : projectiles) shot.x = shot.x * scale + offset;
}

void connectPlayerCombatToEnemies(PlayerEnemyAccess access)
{
    enemies = access;
}
const std::vector<PlayerProjectile>& getPlayerProjectiles()
{
    return projectiles;
}

static bool enemiesConnected()
{
    return enemies.getCount && enemies.getTarget && enemies.damage;
}

// Check the whole path so a fast shot can still hit an enemy between frames.
static int findEnemyInPath(int lane, float left, float right)
{
    if (!enemiesConnected()) {
        return -1;
    }
    int closestId = -1;
    float closestX = right + 1.0f;
    for (int i = 0; i < enemies.getCount(); i++) {
        PlayerAttackTarget target;
        if (!enemies.getTarget(i, target) || target.lane != lane) {
            continue;
        }
        float enemyLeft = target.x - target.halfWidth;
        float enemyRight = target.x + target.halfWidth;
        if (enemyRight < left || enemyLeft > right) {
            continue;
        }
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
        if (right > getEnvironment().getLaneEndX()) {
            right = getEnvironment().getLaneEndX();
        }
        int enemyId = findEnemyInPath(shot.lane, oldX - SPRAY_PROJECTILE_RADIUS, right);
        if (enemyId >= 0) {
            playSound(SoundId::SprayHit);
            enemies.damage(enemyId, 1);
            projectiles.erase(projectiles.begin() + i);
        } else if (shot.x - SPRAY_PROJECTILE_RADIUS > getEnvironment().getLaneEndX()) {
            projectiles.erase(projectiles.begin() + i);
        }
    }
}

void updatePlayerCombat(float dt)
{
    if (!isGameplayRunning() || dt <= 0.0f || !std::isfinite(dt)) {
        return;
    }

    // Split long frames so attack timers and shots advance together.
    while (dt > 0.0f) {
        float step = dt;
        if (step > MAX_COMBAT_STEP) {
            step = MAX_COMBAT_STEP;
        }
        std::vector<Player>& players = getPlayers();
        for (int i = 0; i < (int)players.size(); i++) {
            Player& player = players[i];
            player.attackAnimation -= step;
            if (player.attackAnimation < 0.0f) {
                player.attackAnimation = 0.0f;
            }
            if (player.type == PLAYER_DEFENSE || player.hp <= 0) {
                continue;
            }
            player.attackCooldown -= step;
            if (player.attackCooldown > 0.0f) {
                continue;
            }

            player.attackAnimation = PLAYER_ATTACK_ANIMATION_TIME;
            if (player.type == PLAYER_MELEE) {
                playSound(SoundId::SwatterSwing);
                player.attackCooldown += MELEE_ATTACK_INTERVAL;
                float attackEndX = player.x + PLAYER_HALF_WIDTH + MELEE_ATTACK_REACH;
                int enemyId = findEnemyInPath(player.lane, player.x, attackEndX);
                if (enemyId >= 0) {
                    playSound(SoundId::SwatterHit);
                    enemies.damage(enemyId, 1);
                }
            } else if (player.type == PLAYER_RANGED) {
                player.attackCooldown += SPRAY_ATTACK_INTERVAL;
                // Begin just inside the front hitbox so a touching enemy is hit.
                float shotX = player.x + PLAYER_HALF_WIDTH * 0.8f;
                float shotY = player.y + 0.048f;
                projectiles.push_back({player.lane, shotX, shotY});
                playSound(SoundId::SprayFire);
            }
        }
        advanceProjectiles(step);
        dt -= step;
    }
}

int getPlayerDamageReceiver(int playerIndex, float attackerX)
{
    const std::vector<Player>& players = getPlayers();
    if (playerIndex < 0 || playerIndex >= (int)players.size()) {
        return -1;
    }
    const Player& ally = players[playerIndex];
    if (attackerX < ally.x) {
        return playerIndex;
    }
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
    if (!isGameplayRunning()) {
        return;
    }
    int receiver = getPlayerDamageReceiver(playerIndex, attackerX);
    if (receiver >= 0) {
        damagePlayer(receiver, damage);
    }
}

void resetPlayerCombat()
{
    projectiles.clear();
    for (Player& player : getPlayers()) {
        player.attackCooldown = 0.0f;
        player.attackAnimation = 0.0f;
    }
}

void drawPlayerCombat(GLint t, GLint c, int firstCircle, int circleCount)
{
    for (const PlayerProjectile& shot : projectiles) {
        drawIntroImage(t,c,"projectile-spray",shot.x,shot.y,.10f,.10f);
    }
}
