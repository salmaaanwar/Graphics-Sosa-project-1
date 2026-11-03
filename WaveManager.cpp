#include "WaveManager.h"
#include "Enemy.h"
#include "Environment.h"
#include "Game.h"
#include "Renderer.h"
#include "UI.h"
#include <string>
#include <cstdlib>

static constexpr int WAVE_SIZES[] = {3, 5, 10};
static constexpr int WAVE_COUNT = sizeof(WAVE_SIZES) / sizeof(WAVE_SIZES[0]);
static constexpr float SPAWN_INTERVAL = 1.2f;
static constexpr float WAVE_ANNOUNCEMENT_TIME = 2.0f;

static int waveIndex = 0;
static int enemiesSpawned = 0;
static double timeUntilSpawn = WAVE_ANNOUNCEMENT_TIME;

void resetWaves()
{
    waveIndex = 0;
    enemiesSpawned = 0;
    timeUntilSpawn = WAVE_ANNOUNCEMENT_TIME;
}

void updateWaves(float deltaTime)
{
    if (!isGameplayRunning() || waveIndex >= WAVE_COUNT)
    {
        return;
    }

    // The next wave waits until every enemy from this one is gone.
    if (enemiesSpawned == WAVE_SIZES[waveIndex])
    {
        if (countEnemies() == 0)
        {
            ++waveIndex;
            enemiesSpawned = 0;
            timeUntilSpawn = WAVE_ANNOUNCEMENT_TIME;
            if (waveIndex == WAVE_COUNT)
            {
                finishGame(true);
            }
        }
        return;
    }

    timeUntilSpawn -= deltaTime;
    if (timeUntilSpawn > 0.0f)
    {
        return;
    }

    int enemyType = ENEMY_EASY;
    if (waveIndex == 2 && enemiesSpawned % 3 == 0)
    {
        enemyType = ENEMY_HARD;
    }
    else if (waveIndex >= 1 && enemiesSpawned % 2 != 0)
    {
        enemyType = ENEMY_MEDIUM;
    }

    const int lane = std::rand() % getEnvironment().getLaneCount();
    spawnEnemy(enemyType, lane);
    ++enemiesSpawned;
    timeUntilSpawn = SPAWN_INTERVAL;
}

void drawWaveStatus(GLint transformLocation, GLint colorLocation)
{
    if (!isGameplayRunning() || waveIndex >= WAVE_COUNT || enemiesSpawned != 0)
    {
        return;
    }

    glUniform3f(colorLocation, 0.85f, 0.72f, 0.45f);
    drawRectangle(transformLocation, 0.0f, 0.57f, 0.88f, 0.15f);
    glUniform3f(colorLocation, 0.16f, 0.20f, 0.28f);
    drawRectangle(transformLocation, 0.0f, 0.57f, 0.85f, 0.12f);
    glUniform3f(colorLocation, 1.0f, 0.92f, 0.30f);
    drawCenteredText(transformLocation, "WAVE " + std::to_string(waveIndex + 1) + " STARTING",
                     0.0f, 0.57f, 0.008f);
}
