#include "Game.h"
#include "Cutscene.h"
#include "UI.h"
#include <cmath>
#include <cstdlib>
#include <iostream>

static void check(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

static bool near(float a, float b)
{
    return std::fabs(a - b) < 0.001f;
}

static void testIntroAndStartButton()
{
    returnToMainMenu();
    updateGame(30.0f);
    check(getGameState() == STATE_MAIN_MENU, "main menu waits for Start");
    check(near(getCutsceneTime(), 0.0f), "menu does not advance the intro");

    // Exercise the actual menu handler at the Start button's world position.
    handleUIMouseClick(nullptr, 0.0f, 0.02f);
    check(getGameState() == STATE_CUTSCENE, "Start goes to the intro");
    updateGame(8.0f);
    handleUIMouseClick(nullptr, 0.0f, -0.42f);
    handleUIKeyPress(nullptr, GLFW_KEY_ESCAPE, GLFW_PRESS);
    check(getGameState() == STATE_CUTSCENE, "old Continue click and Escape do not skip the intro");
    updateGame(7.9f);
    check(getGameState() == STATE_CUTSCENE, "intro lasts at least 16 seconds");
    updateGame(0.2f);
    check(getGameState() == STATE_PLAYING, "intro ends automatically");
    check(near(getCutsceneTime(), CUTSCENE_DURATION), "intro time is clamped at its end");
    check(near(getGameplayTime(), 0.0f), "intro time is not counted as gameplay time");
}

static void testFrameRates()
{
    for (int fps : {30, 60, 144})
    {
        startNewGame();
        float step = 1.0f / fps;
        int frames = 0;
        while (getGameState() == STATE_CUTSCENE && frames < fps * 17)
        {
            updateGame(step);
            frames++;
        }
        float seconds = frames * step;
        check(getGameState() == STATE_PLAYING, "intro completes at different frame rates");
        check(std::fabs(seconds - CUTSCENE_DURATION) <= step * 1.1f,
            "intro duration differs by no more than one rendered frame");
    }
}

static void testPauseAndRestart()
{
    restartGame();
    updateGame(2.0f);
    setCollectibles(90);
    handleUIMouseClick(nullptr, -0.74f, 0.84f);
    check(getSelectedCard() == 0, "test setup selects the melee card");

    handleUIKeyPress(nullptr, GLFW_KEY_ESCAPE, GLFW_PRESS);
    check(getGameState() == STATE_PAUSED, "Escape pauses gameplay");
    updateGame(50.0f);
    handleUIKeyPress(nullptr, GLFW_KEY_UP, GLFW_PRESS);
    check(near(getGameplayTime(), 2.0f), "pause freezes gameplay time");
    check(getCollectibles() == 90, "paused UI ignores gameplay input");
    handleUIMouseClick(nullptr, 0.0f, 0.10f);
    check(isGameplayRunning(), "Continue resumes gameplay");
    check(getCollectibles() == 90 && getSelectedCard() == 0, "Continue preserves the session");
    updateGame(1.0f);
    check(near(getGameplayTime(), 3.0f), "resumed time has no pause jump");

    handleUIMouseClick(nullptr, 0.78f, 0.84f);
    check(getGameState() == STATE_PAUSED, "HUD pause button uses the same transition");
    handleUIMouseClick(nullptr, 0.0f, -0.14f);
    check(isGameplayRunning(), "pause-menu Restart goes directly to gameplay");
    check(near(getGameplayTime(), 0.0f), "Restart clears gameplay time");
    check(getCollectibles() == 30 && getSelectedCard() == -1, "Restart resets placeholder data");

    pauseGame();
    handleUIMouseClick(nullptr, 0.0f, -0.38f);
    check(getGameState() == STATE_MAIN_MENU, "pause-menu Main Menu returns to opening screen");
}

static void testResults()
{
    for (bool won : {true, false})
    {
        restartGame();
        updateGame(3.0f);
        finishGame(won);
        check(getGameState() == (won ? STATE_WON : STATE_LOST), "gameplay result selects the correct menu");
        updateGame(20.0f);
        handleUIKeyPress(nullptr, GLFW_KEY_UP, GLFW_PRESS);
        check(near(getGameplayTime(), 3.0f), "finished games stop updating");
        check(getCollectibles() == 30, "finished games ignore gameplay input");
        resumeGame();
        check(!isGameplayRunning(), "Continue cannot resume a finished game");
        handleUIMouseClick(nullptr, 0.0f, -0.04f);
        check(isGameplayRunning() && near(getGameplayTime(), 0.0f), "result-menu Restart resets the session");

        finishGame(won);
        handleUIMouseClick(nullptr, 0.0f, -0.32f);
        check(getGameState() == STATE_MAIN_MENU, "result-menu Main Menu works");
    }
}

static void testFreshIntroAndGuards()
{
    startNewGame();
    updateGame(6.0f);
    startNewGame();
    check(near(getCutsceneTime(), 0.0f), "a new Start resets an unfinished intro");
    updateGame(-5.0f);
    updateGame(0.0f);
    check(near(getCutsceneTime(), 0.0f), "non-positive delta time is ignored");
    setGameState(99);
    check(getGameState() == STATE_CUTSCENE, "unknown states are rejected");
    pauseGame();
    finishGame(false);
    check(getGameState() == STATE_CUTSCENE, "gameplay events cannot interrupt the intro");
    updateGame(100.0f);
    check(isGameplayRunning() && near(getCutsceneTime(), CUTSCENE_DURATION), "a long frame completes the intro safely");
}

int main()
{
    testIntroAndStartButton();
    testFrameRates();
    testPauseAndRestart();
    testResults();
    testFreshIntroAndGuards();
    std::cout << "Game flow tests passed: intro timing, menu clicks, pause, restart, results and resets.\n";
    return 0;
}
