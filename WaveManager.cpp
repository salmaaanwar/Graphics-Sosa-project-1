#include "Audio.h"
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

// Seconds between enemies in waves 1, 2 and 3.
static constexpr float SPAWN_INTERVALS[] = {
    3.5f,
    4.5f,
    5.5f
};

static constexpr float WAVE_ANNOUNCEMENT_TIME = 2.0f;

static int waveIndex = 0;
static bool announcementPlayed = false;
static int enemiesSpawned = 0;
static double timeUntilSpawn = WAVE_ANNOUNCEMENT_TIME;

void resetWaves()
{
    waveIndex = 0;
    announcementPlayed = false;
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
            announcementPlayed = false;
            timeUntilSpawn = WAVE_ANNOUNCEMENT_TIME;
            if (waveIndex == WAVE_COUNT)
            {
                finishGame(true);
            }
        }
        return;
    }

    if (!announcementPlayed) {
        playSound(SoundId::Wave);
        announcementPlayed = true;
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

    // Use each lane in turn before repeating.
    const int lane = (enemiesSpawned + waveIndex) % getEnvironment().getLaneCount();
    spawnEnemy(enemyType, lane);
    ++enemiesSpawned;
    timeUntilSpawn = SPAWN_INTERVALS[waveIndex];
}

void drawWaveStatus(GLint transformLocation, GLint colorLocation)
{
    if (!isGameplayRunning() || waveIndex >= WAVE_COUNT || enemiesSpawned != 0)
    {
        return;
    }

    drawIntroImage(transformLocation, colorLocation, "wave-" + std::to_string(waveIndex + 1),
                   0, .57f, .88f, .15f);
}
