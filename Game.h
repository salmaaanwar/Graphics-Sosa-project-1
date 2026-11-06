#pragma once

class Environment;
Environment& getEnvironment();

const int STATE_MAIN_MENU = 0;
const int STATE_CUTSCENE  = 1;
const int STATE_PLAYING   = 2;
const int STATE_PAUSED    = 3;
const int STATE_WON       = 4;
const int STATE_LOST      = 5;

int getGameState();
void setGameState(int state);

void startNewGame();
void restartGame();
void resetGame();
void returnToMainMenu();
void pauseGame();
void resumeGame();
void finishGame(bool playerWon);

// deltaTime is the number of seconds since the previous frame, like Lab 4.
void updateGame(float deltaTime);
bool isGameplayRunning();
float getGameplayTime();
