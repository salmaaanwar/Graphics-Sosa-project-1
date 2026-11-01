#include "Game.h"
#include "Cutscene.h"
#include "Environment.h"
#include "Player.h"
#include "PlayerCombat.h"
#include "Deployment.h"
#include "UI.h"
#include <cmath>
#include "Enemy.h"

// Only this file stores the state. UI buttons request transitions through functions.
static int currentGameState = STATE_MAIN_MENU;
static float gameplayTime = 0.0f;
static Environment environment;

Environment &getEnvironment()
{
    return environment;
}

int getGameState()
{
    return currentGameState;
}

void setGameState(int state)
{
    if (state < STATE_MAIN_MENU || state > STATE_LOST)
        return;

    // Entering the intro always starts its animation from the beginning.
    if (state == STATE_CUTSCENE)
        resetCutscene();
    currentGameState = state;
}

void startNewGame()
{
    environment.reset();
    resetDeployment();
    resetPlayers();
    resetEnemies();

    // temporary enemy test
    spawnEnemy(ENEMY_EASY, 0);
    spawnEnemy(ENEMY_MEDIUM, 1);
    spawnEnemy(ENEMY_HARD, 2);

    resetPlayerCombat();
    gameplayTime = 0.0f;
    setGameState(STATE_CUTSCENE);
}

void restartGame()
{
    environment.reset();
    resetDeployment();
    resetPlayers();
    resetEnemies();

    // temporary enemy test
    spawnEnemy(ENEMY_EASY, 0);
    spawnEnemy(ENEMY_MEDIUM, 1);
    spawnEnemy(ENEMY_HARD, 2);

    resetPlayerCombat();
    resetCutscene();
    gameplayTime = 0.0f;
    // A retry starts gameplay directly; Start from the main menu plays the intro.
    setGameState(STATE_PLAYING);
}

void returnToMainMenu()
{
    environment.reset();
    resetDeployment();
    resetPlayers();
    resetEnemies();
    resetPlayerCombat();
    resetCutscene();
    gameplayTime = 0.0f;
    setGameState(STATE_MAIN_MENU);
}

void pauseGame()
{
    if (currentGameState == STATE_PLAYING)
        setGameState(STATE_PAUSED);
}

void resumeGame()
{
    if (currentGameState == STATE_PAUSED)
        setGameState(STATE_PLAYING);
}

void finishGame(bool playerWon)
{
    if (currentGameState != STATE_PLAYING)
        return;
    setGameState(playerWon ? STATE_WON : STATE_LOST);
}

bool isGameplayRunning()
{
    return currentGameState == STATE_PLAYING;
}

float getGameplayTime()
{
    return gameplayTime;
}

void updateGame(float deltaTime)
{
    if (deltaTime <= 0.0f || !std::isfinite(deltaTime))
        return;

    if (currentGameState == STATE_CUTSCENE)
    {
        updateCutscene(deltaTime);
        if (isCutsceneFinished())
            setGameState(STATE_PLAYING);
    }
    else if (currentGameState == STATE_PLAYING)
    {
        gameplayTime += deltaTime;
        // Movement and attacks must share the same time steps: otherwise a slow
        // frame fires every shot from the player's final position for that frame.
        while (deltaTime > 0.0f)
        {
            float step = deltaTime > 1.0f / 120.0f ? 1.0f / 120.0f : deltaTime;
            environment.update(step);
            updateEnemies(step);

            if(!isGameplayRunning()){
                break;
            }

            updatePlayers(step);
            updatePlayerCombat(step);
            deltaTime -= step;
        }
    }
}
