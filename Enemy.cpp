#include "Enemy.h"
#include "Environment.h"
#include "Game.h"
#include "Renderer.h"
#include "UI.h"
#include "Player.h"
#include "PlayerCombat.h"
#include "Collectibles.h"

// active enemies
static std::vector<Enemy> enemies;
static std::vector<EnemyProjectile> enemyProjectiles;
static int nextEnemyId = 0;
static void damageEnemyById(int enemyId, int amount);
static void connectEnemyCombat()
{
    connectPlayerCombatToEnemies({countEnemies, readEnemyTarget, damageEnemyById});
}

// access the original enemy list
std::vector<Enemy> &getEnemies()
{
    return enemies;
}

// remove all enemies
void resetEnemies()
{
    enemies.clear();
    enemyProjectiles.clear();
    nextEnemyId = 0;
}

// create one enemy
int spawnEnemy(int type, int lane)
{
    connectEnemyCombat();
    Environment &env = getEnvironment();

    // reject invalid enemy types or lanes
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
    for (int i = 0; i < (int)enemies.size(); ++i) {
        if (enemies[i].id != enemyId) continue;
        enemies[i].hp -= amount;
        if (enemies[i].hp <= 0) { dropCollectible(enemies[i].x, enemies[i].y); enemies.erase(enemies.begin() + i); }
        return;
    }
}

void damageEnemy(int enemyId, int amount) { damageEnemyById(enemyId, amount); }

// move bullets and check player hits
void updateEnemyProjectiles(float dt)
{
    for (int i = (int)enemyProjectiles.size() - 1; i >= 0; i--)
    {
        float oldX = enemyProjectiles[i].x;
        enemyProjectiles[i].x -= 0.70f * dt;
        float newX = enemyProjectiles[i].x;

        int hitPlayer = -1;

        for (int j = 0; j < (int)getPlayers().size(); j++)
        {
            if (getPlayers()[j].lane != enemyProjectiles[i].lane)
            {
                continue;
            }

            // check defense protection before checking dodging
            int receiver = getPlayerDamageReceiver(j, oldX);

            if (receiver < 0)
            {
                continue;
            }

            bool dodged = false;

            if (enemyProjectiles[i].type == ENEMY_EASY && getPlayers()[receiver].pose == POSE_DUCKING)
            {
                dodged = true;
            }

            if (enemyProjectiles[i].type == ENEMY_MEDIUM && getPlayers()[receiver].pose == POSE_JUMPING)
            {
                dodged = true;
            }

            if (dodged)
            {
                continue;
            }

            float playerX = getPlayers()[receiver].x;
            float contactDistance = PLAYER_HALF_WIDTH + 0.012f;

            if (oldX >= playerX - contactDistance && newX <= playerX + contactDistance)
            {
                // choose the first player along the bullet's path
                if (hitPlayer == -1)
                {
                    hitPlayer = receiver;
                }
                else if (playerX > getPlayers()[hitPlayer].x)
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

    for (int i = 0; i < (int)enemies.size(); i++)
    {
        enemies[i].blockedByPlayer = false;

        // check players in the same lane
        for (int j = 0; j < (int)getPlayers().size(); j++)
        {
            if (enemies[i].lane == getPlayers()[j].lane)
            {
                float distance = enemies[i].x - getPlayers()[j].x;

                float contactDistance = ENEMY_HALF_WIDTH + PLAYER_HALF_WIDTH;

                if (distance >= 0.0f && distance <= contactDistance)
                {
                    enemies[i].blockedByPlayer = true;
                    setPlayerBlocked(j, true);
                }
            }
        }

        float speed = 0.10f;

        if (enemies[i].type == ENEMY_HARD)
        {
            speed = 0.18f;
        }

        if (!enemies[i].blockedByPlayer)
        {
            enemies[i].x -= speed * dt;
        }

        // reaching the food ends the game
        float minimumX = getEnvironment().getHomeBaseRightEdgeX() + ENEMY_HALF_WIDTH;

        if (enemies[i].x <= minimumX)
        {
            enemies[i].x = minimumX;
            finishGame(false);
            return;
        }

        // count down the attack animation
        enemies[i].attackAnimation -= dt;

        if (enemies[i].attackAnimation < 0.0f)
        {
            enemies[i].attackAnimation = 0.0f;
        }

        // automatic ranged attacks
        if (enemies[i].type == ENEMY_EASY ||
            enemies[i].type == ENEMY_MEDIUM)
        {

            enemies[i].attackCooldown -= dt;

            if (enemies[i].attackCooldown <= 0.0f)
            {
                EnemyProjectile bullet;

                bullet.type = enemies[i].type;
                bullet.lane = enemies[i].lane;
                bullet.x = enemies[i].x - 0.08f;

                if (enemies[i].type == ENEMY_EASY)
                {
                    bullet.y = enemies[i].y + 0.045f;
                }
                else
                {
                    bullet.y = enemies[i].y - 0.03f;
                }

                enemyProjectiles.push_back(bullet);
                enemies[i].attackAnimation = 0.20f;
                enemies[i].attackCooldown += 1.0f;
            }
        }
        // cockroach melee attack
        if (enemies[i].type == ENEMY_HARD)
        {
            enemies[i].attackCooldown -= dt;

            if (enemies[i].attackCooldown <= 0.0f)
            {
                int target = -1;

                for (int j = 0; j < (int)getPlayers().size(); j++)
                {
                    if (getPlayers()[j].lane == enemies[i].lane)
                    {
                        float distance = enemies[i].x - getPlayers()[j].x;
                        float attackDistance =
                            ENEMY_HALF_WIDTH + PLAYER_HALF_WIDTH;

                        if (distance >= 0.0f &&
                            distance <= attackDistance)
                        {

                            if (target == -1)
                            {
                                target = j;
                            }
                            else if (getPlayers()[j].x >
                                     getPlayers()[target].x)
                            {
                                target = j;
                            }
                        }
                    }
                }

                if (target != -1)
                {
                    damagePlayerFromEnemy(target, 2, enemies[i].x);
                }
                enemies[i].attackAnimation = 0.20f;
                enemies[i].attackCooldown += 0.9f;
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
// draw enemy models
void drawEnemies(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart)
{
    for (int i = 0; i < (int)enemies.size(); i++)
    {
        drawHealthBar(transformLocation, colorLocation, enemies[i].x, enemies[i].y + 0.12f, enemies[i].hp, enemies[i].maxHp);
        float redness = 1.0f - (float)enemies[i].hp / enemies[i].maxHp;
        glUniform3f(colorLocation, 0.75f + 0.25f * redness, 0.75f * (1.0f - redness), 0.75f * (1.0f - redness));
        if (enemies[i].type == ENEMY_EASY)
        {
            float recoil = enemies[i].attackAnimation * 0.06f;

            drawMenuInsect(
                transformLocation,
                colorLocation,
                circleVertexStart,
                circleVertexCount,
                triangleVertexStart,
                enemies[i].x + recoil,
                enemies[i].y + 0.05f,
                0.11f);

            // muzzle flash while firing
            if (enemies[i].attackAnimation > 0.0f)
            {
                glUniform3f(colorLocation, 1.0f, 0.25f, 0.05f);

                drawCircle(
                    transformLocation,
                    circleVertexStart,
                    circleVertexCount,
                    enemies[i].x + recoil - 0.08f,
                    enemies[i].y + 0.045f,
                    0.02f);
            }
        }
        else if (enemies[i].type == ENEMY_MEDIUM)
        {
            drawAnt(
                transformLocation,
                colorLocation,
                circleVertexStart,
                circleVertexCount,
                enemies[i].x,
                enemies[i].y,
                enemies[i].attackAnimation);
        }

        else if (enemies[i].type == ENEMY_HARD)
        {
            drawCockroach(
                transformLocation,
                colorLocation,
                circleVertexStart,
                circleVertexCount,
                triangleVertexStart,
                enemies[i].x,
                enemies[i].y,
                enemies[i].attackAnimation);
        }

        float damageRed = 1.0f - (float)enemies[i].hp / enemies[i].maxHp;
        if (damageRed > 0.0f) {
            glUniform3f(colorLocation, 0.85f, 0.12f, 0.10f);
            drawCircle(transformLocation, circleVertexStart, circleVertexCount,
                       enemies[i].x, enemies[i].y, 0.008f + damageRed * 0.015f);
        }
    }

    for (int i = 0; i < (int)enemyProjectiles.size(); i++)
    {
        if (enemyProjectiles[i].type == ENEMY_EASY)
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
            enemyProjectiles[i].x,
            enemyProjectiles[i].y,
            0.012f);
    }
}

// number of enemies in the list
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
