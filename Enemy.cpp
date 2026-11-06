#include "Audio.h"
#include "Enemy.h"
#include <algorithm>
#include <string>
#include "Environment.h"
#include "Game.h"
#include "Renderer.h"
#include "UI.h"
#include "Player.h"
#include "PlayerCombat.h"
#include "Collectibles.h"

static std::vector<Enemy> enemies;
static std::vector<EnemyProjectile> enemyProjectiles;

void resizeEnemyPositions(float scale, float offset)
{
    for (Enemy& enemy : enemies) enemy.x = enemy.x * scale + offset;
    for (EnemyProjectile& shot : enemyProjectiles) shot.x = shot.x * scale + offset;
}
static int nextEnemyId = 0;
static constexpr float SHOT_RECOVERY_TIME = 0.9f;
static float shotRecovery = SHOT_RECOVERY_TIME;
static void damageEnemyById(int enemyId, int amount);
static void connectEnemyCombat()
{
    connectPlayerCombatToEnemies({countEnemies, readEnemyTarget, damageEnemyById});
}

std::vector<Enemy> &getEnemies()
{
    return enemies;
}

void resetEnemies()
{
    shotRecovery = SHOT_RECOVERY_TIME;
    enemies.clear();
    enemyProjectiles.clear();
    nextEnemyId = 0;
}

int spawnEnemy(int type, int lane)
{
    connectEnemyCombat();
    Environment &env = getEnvironment();

    if (type < ENEMY_EASY || type > ENEMY_HARD)
    {
        return -1;
    }

    if (lane < 0 || lane >= env.getLaneCount())
    {
        return -1;
    }

    Enemy enemy;

    enemy.id = nextEnemyId;
    enemy.type = type;
    enemy.lane = lane;

    enemy.x = env.getLaneEndX() - 0.06f;
    enemy.y = env.getLaneCenterY(lane) - 0.015f;

    if (type == ENEMY_HARD)
    {
        enemy.maxHp = 7;
    }
    else
    {
        enemy.maxHp = 5;
    }

    enemy.hp = enemy.maxHp;
    enemy.blockedByPlayer = false;
    enemy.attackCooldown = 0.0f;
    enemy.attackAnimation = 0.0f;

    enemies.push_back(enemy);
    playSound(SoundId::Approaching);
    nextEnemyId++;

    return enemy.id;
}

static void damageEnemyById(int enemyId, int amount)
{
    for (auto enemy = enemies.begin(); enemy != enemies.end(); ++enemy)
    {
        if (enemy->id != enemyId)
        {
            continue;
        }

        enemy->hp -= amount;
        if (enemy->hp <= 0)
        {
            int coinValue = 10;
            if (enemy->type == ENEMY_MEDIUM) {
                coinValue = 15;
            } else if (enemy->type == ENEMY_HARD) {
                coinValue = 20;
            }
            playSound(SoundId::InsectDefeated);
            dropCollectible(enemy->x, enemy->y, coinValue);
            enemies.erase(enemy);
        }
        return;
    }
}

void damageEnemy(int enemyId, int amount)
{
    damageEnemyById(enemyId, amount);
}

void updateEnemyProjectiles(float dt)
{
    for (int i = (int)enemyProjectiles.size() - 1; i >= 0; i--)
    {
        EnemyProjectile& projectile = enemyProjectiles[i];
        const float oldX = projectile.x;
        projectile.x -= 0.70f * dt;
        const float newX = projectile.x;
        const auto& players = getPlayers();

        int hitPlayer = -1;

        for (int j = 0; j < (int)players.size(); j++)
        {
            if (players[j].lane != projectile.lane)
            {
                continue;
            }

            // A sheet takes the hit even when the ally behind it could dodge.
            int receiver = getPlayerDamageReceiver(j, oldX);

            if (receiver < 0)
            {
                continue;
            }

            const int pose = players[receiver].pose;
            const bool dodged = (projectile.type == ENEMY_EASY && pose == POSE_DUCKING) ||
                                (projectile.type == ENEMY_MEDIUM && pose == POSE_JUMPING);

            if (dodged)
            {
                continue;
            }

            float playerX = players[receiver].x;
            float contactDistance = PLAYER_HALF_WIDTH + 0.012f;

            if (oldX >= playerX - contactDistance && newX <= playerX + contactDistance)
            {
                // choose the first player along the bullet's path
                if (hitPlayer == -1 || playerX > players[hitPlayer].x)
                {
                    hitPlayer = receiver;
                }
            }
        }

        if (hitPlayer != -1)
        {
            damagePlayer(hitPlayer, 1);
            enemyProjectiles.erase(enemyProjectiles.begin() + i);
        }
        else if (newX < getEnvironment().getLaneStartX() - 0.10f)
        {
            enemyProjectiles.erase(enemyProjectiles.begin() + i);
        }
    }
}

void updateEnemies(float dt)
{
    if (dt <= 0.0f)
    {
        return;
    }

    updateEnemyProjectiles(dt);

    if (!enemyProjectiles.empty()) {
        shotRecovery = SHOT_RECOVERY_TIME;
    } else if (shotRecovery > 0.0f) {
        shotRecovery -= dt;
    }

    clearAllPlayerBlocks();

    const auto& players = getPlayers();
    for (Enemy& enemy : enemies)
    {
        enemy.blockedByPlayer = false;

        for (int j = 0; j < (int)players.size(); j++)
        {
            if (enemy.lane == players[j].lane)
            {
                float distance = enemy.x - players[j].x;

                float contactDistance = ENEMY_HALF_WIDTH + PLAYER_HALF_WIDTH;

                if (distance >= 0.0f && distance <= contactDistance)
                {
                    enemy.blockedByPlayer = true;
                    setPlayerBlocked(j, true);
                }
            }
        }

        float speed = 0.05f;

        if (enemy.type == ENEMY_HARD)
        {
            speed = 0.09f;
        }

        if (!enemy.blockedByPlayer)
        {
            enemy.x -= speed * dt;
        }

        // reaching the food ends the game
        float minimumX = getEnvironment().getHomeBaseRightEdgeX() + ENEMY_HALF_WIDTH;

        if (enemy.x <= minimumX)
        {
            enemy.x = minimumX;
            playSound(SoundId::FoodReached);
            finishGame(false);
            return;
        }

        enemy.attackAnimation -= dt;

        if (enemy.attackAnimation < 0.0f)
        {
            enemy.attackAnimation = 0.0f;
        }

        // automatic ranged attacks
        if (enemy.type == ENEMY_EASY ||
            enemy.type == ENEMY_MEDIUM)
        {
            enemy.attackCooldown -= dt;

            if (enemy.attackCooldown <= 0.0f && enemyProjectiles.empty() && shotRecovery <= 0.0f)
            {
                EnemyProjectile bullet;

                bullet.type = enemy.type;
                bullet.lane = enemy.lane;
                bullet.x = enemy.x - 0.08f;

                if (enemy.type == ENEMY_EASY)
                {
                    bullet.y = enemy.y + 0.045f;
                }
                else
                {
                    bullet.y = enemy.y - 0.03f;
                }

                enemyProjectiles.push_back(bullet);
                shotRecovery = SHOT_RECOVERY_TIME;

                enemy.attackAnimation = 0.20f;
                enemy.attackCooldown = 2.0f;
            }
        }
        // cockroach melee attack
        if (enemy.type == ENEMY_HARD)
        {
            enemy.attackCooldown -= dt;

            if (enemy.attackCooldown <= 0.0f)
            {
                int target = -1;

                for (int j = 0; j < (int)players.size(); j++)
                {
                    if (players[j].lane == enemy.lane)
                    {
                        float distance = enemy.x - players[j].x;
                        float attackDistance =
                            ENEMY_HALF_WIDTH + PLAYER_HALF_WIDTH;

                        if (distance >= 0.0f &&
                            distance <= attackDistance)
                        {
                            if (target == -1 || players[j].x > players[target].x)
                            {
                                target = j;
                            }
                        }
                    }
                }

                if (target != -1)
                {
                    damagePlayerFromEnemy(target, 2, enemy.x);
                }
                enemy.attackAnimation = 0.20f;
                enemy.attackCooldown += 0.9f;
            }
        }
    }
}

void drawEnemies(
    GLint transformLocation,
    GLint colorLocation,
    int,
    int,
    int)
{
    for (const Enemy& enemy : enemies)
    {
        drawHealthBar(transformLocation, colorLocation, enemy.x, enemy.y + 0.12f, enemy.hp, enemy.maxHp);
        float redness = 1.0f - (float)enemy.hp / enemy.maxHp;
        int frame = enemy.attackAnimation > 0
            ? 1 + std::min(3, int(enemy.attackAnimation / .20f * 4))
            : 0;
        std::string image = "enemy-" + std::to_string(enemy.type) + "-" + std::to_string(frame);
        drawCharacterImage(transformLocation, colorLocation, image, enemy.x, enemy.y, redness);
    }

    for (const EnemyProjectile& projectile : enemyProjectiles)
    {
        int imageType = projectile.type == ENEMY_EASY ? 0 : 1;
        drawIntroImage(transformLocation, colorLocation, "projectile-enemy-" + std::to_string(imageType),
                       projectile.x, projectile.y, .10f, .10f);
    }
}

int countEnemies()
{
    return (int)enemies.size();
}

bool readEnemyTarget(int index, PlayerAttackTarget &target)
{
    if (index < 0 || index >= (int)enemies.size())
    {
        return false;
    }

    if (enemies[index].hp <= 0)
    {
        return false;
    }

    target.id = enemies[index].id;
    target.lane = enemies[index].lane;
    target.x = enemies[index].x;
    target.halfWidth = ENEMY_HALF_WIDTH;

    return true;
}
