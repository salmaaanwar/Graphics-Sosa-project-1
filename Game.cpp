#include "Audio.h"
#include "Game.h"
#include <algorithm>
#include "Cutscene.h"
#include "Environment.h"
#include "Player.h"
#include "PlayerCombat.h"
#include "Deployment.h"
#include "UI.h"
#include <cmath>
#include "Enemy.h"
#include "WaveManager.h"
#include "Collectibles.h"

static constexpr float MAX_SIMULATION_STEP = 1.0f / 120.0f;

static int currentGameState = STATE_MAIN_MENU;
static float gameplayTime = 0.0f;
static Environment environment;

Environment& getEnvironment()
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
    const int previous = currentGameState;
    currentGameState = state;
    audioGameStateChanged(previous, state);
}

void startNewGame()
{
    resetGame();
    setGameState(STATE_CUTSCENE);
}

void resetGame()
{
    clearGameplayAudio();
    environment.reset();
    resetDeployment();
    resetPlayers();
    resetEnemies();
    resetPlayerCombat();
    resetWaves();
    resetCollectibles();
    gameplayTime = 0.0f;
}

void restartGame()
{
    resetGame();
    resetCutscene();
    // A retry always starts the music over, including retries from pause.
    currentGameState = STATE_PLAYING;
    audioGameStateChanged(-1, STATE_PLAYING);
}

void returnToMainMenu()
{
    resetGame();
    resetCutscene();
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
    if (playerWon)
    {
        setGameState(STATE_WON);
    }
    else
    {
        setGameState(STATE_LOST);
    }
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
            float step = deltaTime;
            if (step > MAX_SIMULATION_STEP)
            {
                step = MAX_SIMULATION_STEP;
            }
            environment.update(step);
            updateEnemies(step);

            if (!isGameplayRunning())
            {
                break;
            }

            updatePlayers(step);
            updatePlayerCombat(step);
            updateWaves(step);
            updateCollectibles();
            deltaTime -= step;
        }
    }
}
