#include "Game.h"
#include "Cutscene.h"
#include "Deployment.h"
#include "Environment.h"
#include "Player.h"
#include "PlayerCombat.h"
#include "UI.h"
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <string>

static int failures = 0;
static void check(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        failures++;
    }
}

static bool near(float a, float b)
{
    return std::fabs(a - b) < 0.001f;
}
static void click(float x, float y)
{
    getEnvironment().handleMouseButton(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, x, y);
    getEnvironment().handleMouseButton(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, x, y);
}

static void movement()
{
    for (int fps : {30, 60, 144}) {
        restartGame();
        spawnPlayer(PLAYER_MELEE, 0);
        spawnPlayer(PLAYER_RANGED, 2);
        spawnPlayer(PLAYER_DEFENSE, 3);
        float startX = getPlayers()[0].x;
        for (int frame = 0; frame < fps * 3; frame++) updateGame(1.0f / fps);
        check(near(getPlayers()[0].x, startX + 0.42f) && near(getPlayers()[1].x, startX + 0.42f),
              "melee/ranged movement agrees at 30/60/144 FPS");
        check(near(getPlayers()[2].x, startX), "Defense stays stationary");
        setPlayerBlocked(0, true);
        float stoppedX = getPlayers()[0].x;
        updateGame(0.2f);
        check(near(getPlayers()[0].x, stoppedX), "encounter flag stops movement");
        clearAllPlayerBlocks();
        updateGame(0.2f);
        check(getPlayers()[0].x > stoppedX, "clearing encounter flags resumes movement");
        click(0.0f, getEnvironment().getLaneCenterY(0));
        click(0.0f, getEnvironment().getLaneCenterY(3));
        handlePlayerKeyPress(GLFW_KEY_W, GLFW_PRESS);
        updateGame(0.1f);
        check(getPlayers()[0].pose == POSE_JUMPING && getPlayers()[1].pose == POSE_NORMAL &&
              getPlayers()[2].pose == POSE_NORMAL, "jump affects selected mobile units only");
        updateGame(0.6f);
        check(getPlayers()[0].pose == POSE_NORMAL && near(getPlayers()[0].y,
              getEnvironment().getLaneCenterY(0) - 0.015f), "jump ends at lane baseline");
        updateGame(20.0f);
        check(near(getPlayers()[0].x, getEnvironment().getLaneEndX() - 0.06f), "movement clamps to lane end");
        damagePlayer(0, 4);
        check(getPlayers()[0].hp == 1, "damage reduces the real player HP");
        damagePlayer(0, 1);
        check(getPlayers().size() == 2 && getPlayers()[0].type == PLAYER_RANGED,
              "defeat erases exactly one player without damaging its neighbor");
    }
}

static void boundariesAndStress()
{
    restartGame();
    Environment& env = getEnvironment();
    for (int lane = 0; lane < env.getLaneCount(); lane++) {
        check(env.getLaneAtPoint(env.getLaneStartX(), env.getLaneCenterY(lane)) == lane &&
              env.getLaneAtPoint(env.getLaneEndX(), env.getLaneCenterY(lane)) == lane,
              "both horizontal lane boundaries are selectable");
        check(env.getLaneAtPoint(0.0f, env.getLaneTop(lane)) == lane &&
              env.getLaneAtPoint(0.0f, env.getLaneBottom(lane)) == lane,
              "both vertical lane boundaries are selectable");
        check(env.getLaneAtPoint(env.getLaneStartX() - 0.001f, env.getLaneCenterY(lane)) == -1,
              "home base is not a placement lane");
        if (lane < env.getLaneCount() - 1)
            check(env.getLaneAtPoint(0, (env.getLaneBottom(lane) + env.getLaneTop(lane + 1)) / 2) == -1,
                  "separator gaps cannot spend coins");
    }
    // Keep the seed fixed so a failed purchase sequence can be replayed.
    std::mt19937 random(502);
    int purchases = 0;
    for (int action = 0; action < 10000; action++) {
        if (action % 100 == 0) {
            restartGame();
            setCollectibles(500);
        }
        int type = (int)(random() % 5) - 1;
        int lane = (int)(random() % 6) - 1;
        selectPlayerCard(type);
        if (action % 7 == 0) pauseGame();
        int card = getSelectedCard();
        int oldCoins = getCollectibles();
        size_t oldCount = getPlayers().size();
        bool bought = deploySelectedPlayer(lane);
        check(getPlayers().size() == oldCount + (bought ? 1 : 0), "transaction creates zero or one unit");
        check(getCollectibles() == oldCoins - (bought ? getPlayerCost(card) : 0), "transaction conserves currency");
        if (bought) purchases++;
        resumeGame();
        updateGame(1.0f / 60.0f);
        check(getCollectibles() >= 0, "stress run never overdraws coins");
    }
    check(purchases > 100, "stress run exercised successful purchases");
    std::cout << "Stress: 10000 input sequences, " << purchases << " purchases.\n";
}

static void invalidTime()
{
    for (float dt : {0.0f, -1.0f, std::numeric_limits<float>::quiet_NaN(),
                     std::numeric_limits<float>::infinity()}) {
        restartGame();
        spawnPlayer(PLAYER_MELEE, 0);
        float x = getPlayers()[0].x;
        updateGame(dt);
        check(near(getGameplayTime(), 0.0f) && near(getPlayers()[0].x, x) &&
              std::isfinite(getPlayers()[0].animTime), "invalid time cannot corrupt a running session");
        startNewGame();
        updateGame(dt);
        check(getGameState() == STATE_CUTSCENE && near(getCutsceneTime(), 0),
              "invalid time cannot corrupt or skip the intro");
    }
}

static void longFrame()
{
    restartGame();
    spawnPlayer(PLAYER_RANGED, 0);
    for (int frame = 0; frame < 60; frame++) updateGame(1.0f / 120.0f);
    float referencePlayerX = getPlayers()[0].x;
    float referenceShotX = getPlayerProjectiles()[0].x;
    restartGame();
    spawnPlayer(PLAYER_RANGED, 0);
    updateGame(0.5f);
    check(near(getPlayers()[0].x, referencePlayerX), "a long frame preserves player movement");
    check(near(getPlayerProjectiles()[0].x, referenceShotX),
          "moving shooter fires from the same position during a long frame");
    std::cout << "Shot position: regular=" << referenceShotX << ", long frame="
              << getPlayerProjectiles()[0].x << '\n';
}

static void frozenStates()
{
    for (int state : {STATE_MAIN_MENU, STATE_CUTSCENE, STATE_PAUSED, STATE_WON, STATE_LOST}) {
        restartGame();
        spawnPlayer(PLAYER_RANGED, 0);
        click(0, getEnvironment().getLaneCenterY(0));
        updateGame(0.1f);
        float x = getPlayers()[0].x;
        float shotX = getPlayerProjectiles()[0].x;
        setGameState(state);
        updateGame(0.2f);
        handlePlayerKeyPress(GLFW_KEY_UP, GLFW_PRESS);
        selectPlayerCard(PLAYER_MELEE);
        bool bought = deploySelectedPlayer(0);
        check(!bought && getPlayers().size() == 1 && getCollectibles() == 30 &&
              near(x, getPlayers()[0].x) && near(shotX, getPlayerProjectiles()[0].x) &&
              getPlayers()[0].pose == POSE_NORMAL, "non-playing states freeze player systems and inputs");
    }
}

int main(int argc, char** argv)
{
    std::string name = argc > 1 ? argv[1] : "all";
    if (name == "all" || name == "movement") movement();
    if (name == "all" || name == "stress") boundariesAndStress();
    if (name == "all" || name == "invalid-time") invalidTime();
    if (name == "all" || name == "long-frame") longFrame();
    if (name == "all" || name == "frozen-states") frozenStates();
    std::cout << name << ": " << failures << " failures.\n";
    return failures ? 1 : 0;
}
