#include "Enemy.h"
#include "Environment.h"
#include "Game.h"
#include "Renderer.h"
#include "UI.h"
#include "Player.h"
#include "PlayerCombat.h"
#include "Collectibles.h"

static std::vector<Enemy> enemies;
static std::vector<EnemyProjectile> enemyProjectiles;
static int nextEnemyId = 0;
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

// move bullets and check player hits
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

            // check defense protection before checking dodging
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

// move enemies toward the home base
void updateEnemies(float dt)
{
    if (dt <= 0.0f)
    {
        return;
    }

    updateEnemyProjectiles(dt);

    clearAllPlayerBlocks();

    const auto& players = getPlayers();
    for (Enemy& enemy : enemies)
    {
        enemy.blockedByPlayer = false;

        // check players in the same lane
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
            finishGame(false);
            return;
        }

        // count down the attack animation
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

            if (enemy.attackCooldown <= 0.0f)
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
                enemy.attackAnimation = 0.20f;
                enemy.attackCooldown += 1.0f;
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

// ant with a slingshot
void drawAnt(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    float x,
    float y,
    float attackAnimation)
{
    // legs
    glUniform3f(colorLocation, 0.20f, 0.10f, 0.05f);

    for (int leg = 0; leg < 3; leg++)
    {
        float legX = x - 0.02f + leg * 0.025f;

        drawRectangle(transformLocation,
                      legX, y - 0.03f, 0.008f, 0.05f, 35.0f);

        drawRectangle(transformLocation,
                      legX, y - 0.03f, 0.008f, 0.05f, -35.0f);
    }

    // abdomen, middle body, and head
    glUniform3f(colorLocation, 0.45f, 0.18f, 0.08f);

    drawCircle(transformLocation,
               circleVertexStart, circleVertexCount,
               x + 0.035f, y, 0.028f);

    drawCircle(transformLocation,
               circleVertexStart, circleVertexCount,
               x, y, 0.018f);

    drawCircle(transformLocation,
               circleVertexStart, circleVertexCount,
               x - 0.035f, y, 0.023f);

    // antennae
    glUniform3f(colorLocation, 0.20f, 0.10f, 0.05f);

    drawRectangle(transformLocation,
                  x - 0.05f, y + 0.028f, 0.005f, 0.035f, 30.0f);

    drawRectangle(transformLocation,
                  x - 0.025f, y + 0.028f, 0.005f, 0.035f, -30.0f);

    // eye
    glUniform3f(colorLocation, 0.95f, 0.85f, 0.65f);

    drawCircle(transformLocation,
               circleVertexStart, circleVertexCount,
               x - 0.045f, y + 0.005f, 0.006f);

    // slingshot recoil
    float slingX = x - 0.075f + attackAnimation * 0.08f;

    glUniform3f(colorLocation, 0.35f, 0.16f, 0.05f);

    drawRectangle(transformLocation,
                  slingX, y - 0.07f, 0.012f, 0.04f);

    drawRectangle(transformLocation,
                  slingX - 0.009f, y - 0.045f,
                  0.01f, 0.035f, 30.0f);

    drawRectangle(transformLocation,
                  slingX + 0.009f, y - 0.045f,
                  0.01f, 0.035f, -30.0f);

    // rubber band
    glUniform3f(colorLocation, 0.10f, 0.10f, 0.15f);

    drawRectangle(transformLocation,
                  slingX, y - 0.03f, 0.035f, 0.005f);
}

// cockroach with a claymore
void drawCockroach(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart,
    float x,
    float y,
    float attackAnimation)
{
    // six legs
    glUniform3f(colorLocation, 0.18f, 0.08f, 0.03f);

    for (int leg = 0; leg < 3; leg++)
    {
        float legY = y + 0.025f - leg * 0.025f;

        drawRectangle(transformLocation,
                      x - 0.045f, legY, 0.045f, 0.008f, 25.0f);

        drawRectangle(transformLocation,
                      x + 0.045f, legY, 0.045f, 0.008f, -25.0f);
    }

    // body
    glUniform3f(colorLocation, 0.30f, 0.12f, 0.04f);

    drawCircle(transformLocation,
               circleVertexStart, circleVertexCount,
               x, y - 0.015f, 0.04f);

    drawRectangle(transformLocation,
                  x, y + 0.005f, 0.065f, 0.07f);

    // wing covers
    glUniform3f(colorLocation, 0.55f, 0.28f, 0.10f);

    drawRectangle(transformLocation,
                  x - 0.016f, y, 0.026f, 0.07f, -8.0f);

    drawRectangle(transformLocation,
                  x + 0.016f, y, 0.026f, 0.07f, 8.0f);

    // head and antennae
    glUniform3f(colorLocation, 0.18f, 0.08f, 0.03f);

    drawCircle(transformLocation,
               circleVertexStart, circleVertexCount,
               x - 0.025f, y + 0.04f, 0.023f);

    drawRectangle(transformLocation,
                  x - 0.045f, y + 0.075f, 0.005f, 0.06f, 30.0f);

    drawRectangle(transformLocation,
                  x - 0.015f, y + 0.075f, 0.005f, 0.06f, -20.0f);

    // eye
    glUniform3f(colorLocation, 0.95f, 0.15f, 0.10f);

    drawCircle(transformLocation,
               circleVertexStart, circleVertexCount,
               x - 0.035f, y + 0.045f, 0.006f);

    // sword thrust toward the player
    float swordX = x - 0.075f - attackAnimation * 0.15f;

    // blade and pointed tip
    glUniform3f(colorLocation, 0.70f, 0.80f, 0.90f);

    drawRectangle(transformLocation,
                  swordX, y + 0.015f, 0.018f, 0.10f);

    drawTriangle(transformLocation, triangleVertexStart,
                 swordX, y + 0.075f, 0.018f, 0.02f);

    // guard and handle
    glUniform3f(colorLocation, 0.25f, 0.18f, 0.10f);

    drawRectangle(transformLocation,
                  swordX, y - 0.04f, 0.05f, 0.01f);

    drawRectangle(transformLocation,
                  swordX, y - 0.06f, 0.012f, 0.03f);
}
void drawEnemies(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart)
{
    for (const Enemy& enemy : enemies)
    {
        drawHealthBar(transformLocation, colorLocation, enemy.x, enemy.y + 0.12f, enemy.hp, enemy.maxHp);
        float redness = 1.0f - (float)enemy.hp / enemy.maxHp;
        glUniform3f(colorLocation, 0.75f + 0.25f * redness, 0.75f * (1.0f - redness), 0.75f * (1.0f - redness));
        if (enemy.type == ENEMY_EASY)
        {
            float recoil = enemy.attackAnimation * 0.06f;

            drawMenuInsect(
                transformLocation,
                colorLocation,
                circleVertexStart,
                circleVertexCount,
                triangleVertexStart,
                enemy.x + recoil,
                enemy.y + 0.05f,
                0.11f);

            // muzzle flash while firing
            if (enemy.attackAnimation > 0.0f)
            {
                glUniform3f(colorLocation, 1.0f, 0.25f, 0.05f);

                drawCircle(
                    transformLocation,
                    circleVertexStart,
                    circleVertexCount,
                    enemy.x + recoil - 0.08f,
                    enemy.y + 0.045f,
                    0.02f);
            }
        }
        else if (enemy.type == ENEMY_MEDIUM)
        {
            drawAnt(
                transformLocation,
                colorLocation,
                circleVertexStart,
                circleVertexCount,
                enemy.x,
                enemy.y,
                enemy.attackAnimation);
        }

        else if (enemy.type == ENEMY_HARD)
        {
            drawCockroach(
                transformLocation,
                colorLocation,
                circleVertexStart,
                circleVertexCount,
                triangleVertexStart,
                enemy.x,
                enemy.y,
                enemy.attackAnimation);
        }

        if (redness > 0.0f) {
            glUniform3f(colorLocation, 0.85f, 0.12f, 0.10f);
            drawCircle(transformLocation, circleVertexStart, circleVertexCount,
                       enemy.x, enemy.y, 0.008f + redness * 0.015f);
        }
    }

    for (const EnemyProjectile& projectile : enemyProjectiles)
    {
        if (projectile.type == ENEMY_EASY)
        {
            glUniform3f(colorLocation, 0.30f, 0.05f, 0.40f);
        }
        else
        {
            glUniform3f(colorLocation, 0.15f, 0.15f, 0.18f);
        }

        drawCircle(
            transformLocation,
            circleVertexStart,
            circleVertexCount,
            projectile.x,
            projectile.y,
            0.012f);
    }
}

int countEnemies()
{
    return (int)enemies.size();
}

// provide one enemy's current position for player attacks
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
