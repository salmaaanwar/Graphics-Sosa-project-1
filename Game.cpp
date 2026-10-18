#include "Game.h"
#include "Cutscene.h"
#include "UI.h"

// Only this file stores the state. UI buttons request transitions through functions.
static int currentGameState = STATE_MAIN_MENU;
static float gameplayTime = 0.0f;

int getGameState()
{
    return currentGameState;
}

void setGameState(int state)
{
    if (state < STATE_MAIN_MENU || state > STATE_LOST) return;

    // Entering the intro always starts its animation from the beginning.
    if (state == STATE_CUTSCENE) resetCutscene();
    currentGameState = state;
}

void startNewGame()
{
    resetGamePlaceholders();
    gameplayTime = 0.0f;
    setGameState(STATE_CUTSCENE);
}

void restartGame()
{
    resetGamePlaceholders();
    resetCutscene();
    gameplayTime = 0.0f;
    // A retry starts gameplay directly; Start from the main menu plays the intro.
    setGameState(STATE_PLAYING);
}

void returnToMainMenu()
{
    resetGamePlaceholders();
    resetCutscene();
    gameplayTime = 0.0f;
    setGameState(STATE_MAIN_MENU);
}

void pauseGame()
{
    if (currentGameState == STATE_PLAYING) setGameState(STATE_PAUSED);
}

void resumeGame()
{
    if (currentGameState == STATE_PAUSED) setGameState(STATE_PLAYING);
}

void finishGame(bool playerWon)
{
    if (currentGameState != STATE_PLAYING) return;
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
    if (deltaTime <= 0.0f) return;

    if (currentGameState == STATE_CUTSCENE)
    {
        updateCutscene(deltaTime);
        if (isCutsceneFinished()) setGameState(STATE_PLAYING);
    }
    else if (currentGameState == STATE_PLAYING)
    {
        gameplayTime += deltaTime;
        // Future player/enemy updates go here, so they also stop during pause/loss.
    }
}
