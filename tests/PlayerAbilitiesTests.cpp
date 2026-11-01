#include "Deployment.h"
#include "Environment.h"
#include "Game.h"
#include "Player.h"
#include "PlayerCombat.h"
#include "UI.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>

static void check(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

// Enemy fixtures exist only in this test. Production enemies/HP belong to 3/4.
struct TestEnemy { int id; int lane; float x; int hp; };
static std::vector<TestEnemy> targets;
static int damageEvents = 0;
static int targetCount() { return (int)targets.size(); }
static bool readTarget(int index, PlayerAttackTarget& target)
{
    if (index < 0 || index >= (int)targets.size()) return false;
    const TestEnemy& enemy = targets[index];
    target = {enemy.id, enemy.lane, enemy.x, 0.04f};
    return enemy.hp > 0;
}
static void hitTarget(int id, int damage)
{
    check(damage == 1, "all player attacks deal exactly one damage");
    for (int i = 0; i < (int)targets.size(); i++) {
        if (targets[i].id != id) continue;
        damageEvents++;
        targets[i].hp -= damage;
        if (targets[i].hp <= 0) targets.erase(targets.begin() + i);
        return;
    }
    check(false, "damage uses a live stable enemy ID");
}
static void fresh()
{
    restartGame();
    targets.clear();
    damageEvents = 0;
    connectPlayerCombatToEnemies({targetCount, readTarget, hitTarget});
}
static void clickLane(int lane, bool release = true)
{
    Environment& env = getEnvironment();
    env.handleMouseButton(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0.2f, env.getLaneCenterY(lane));
    if (release) env.handleMouseButton(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0.2f, env.getLaneCenterY(lane));
}

static void testPurchases()
{
    fresh();
    check(getPlayers().empty(), "a new game does not give away starter units");
    check(getCollectibles() == 30 && isCardUnlocked(0) && !isCardUnlocked(1) && !isCardUnlocked(2),
          "30 starting coins unlock only melee");
    check(spawnPlayer(-1, 0) == -1 && spawnPlayer(3, 0) == -1 && spawnPlayer(0, 4) == -1,
          "invalid shared spawn requests are rejected");
    clickLane(0);
    clickLane(2);
    handleUIMouseClick(nullptr, -0.74f, 0.84f);
    check(getSelectedCard() == PLAYER_MELEE && getPlayers().empty(), "selecting a card only arms a purchase");
    check(!deploySelectedPlayer(-1) && !deploySelectedPlayer(4) && getCollectibles() == 30,
          "invalid placement spends nothing");
    clickLane(2, false);
    clickLane(2, false);
    check(getPlayers().size() == 1 && getPlayers()[0].lane == 2 && getCollectibles() == 20,
          "one press creates one unit despite multiple selected lanes and repeated press events");
    clickLane(2);
    check(getSelectedCard() == -1 && getEnvironment().isLaneSelected(2), "purchase keeps lane selected and clears card");
    check(std::fabs(getPlayers()[0].x - (getEnvironment().getLaneStartX() + 0.08f)) < 0.001f,
          "deployment uses Member 1's leftmost spawn position");
    for (int i = 0; i < 2; i++) {
        selectPlayerCard(0);
        check(deploySelectedPlayer(2), "friendly overlap allows all three starter purchases");
    }
    check(getPlayers().size() == 3 && getCollectibles() == 0 && !isCardUnlocked(0),
          "three successful purchases spend the starter budget exactly");
    selectPlayerCard(0);
    check(!deploySelectedPlayer(0) && getPlayers().size() == 3, "insufficient funds cannot create a unit");

    setCollectibles(59);
    check(!isCardUnlocked(1), "spray stays locked below 60");
    setCollectibles(60);
    check(isCardUnlocked(1) && !isCardUnlocked(2), "spray unlocks at twice the starting budget");
    selectPlayerCard(1);
    setCollectibles(59);
    check(getSelectedCard() == -1, "currency reduction immediately clears an unaffordable selection");
    setCollectibles(89);
    check(!isCardUnlocked(2), "sheet stays locked below 90");
    setCollectibles(180);
    selectPlayerCard(2);
    check(deploySelectedPlayer(1), "sheet purchase succeeds at the threshold");
    selectPlayerCard(2);
    check(!deploySelectedPlayer(1) && getCollectibles() == 90 && getPlayers().size() == 4,
          "duplicate stationary placement is rejected before charging");
    check(deploySelectedPlayer(3) && getCollectibles() == 0, "another lane accepts one sheet");
    setCollectibles(-10);
    check(getCollectibles() == 0 && !isCardUnlocked(-1), "currency is clamped and unknown cards are locked");
}

static void testMeleeAndEnemyRemoval()
{
    fresh();
    spawnPlayer(PLAYER_MELEE, 0);
    getPlayers()[0].x = 0.0f;
    targets = {{10, 0, 0.11f, 5}, {11, 1, 0.10f, 5}, {12, 0, -0.3f, 5}, {13, 0, 0.6f, 5}};
    updatePlayerCombat(0.1f);
    check(targets[0].hp == 4 && targets[1].hp == 5 && targets[2].hp == 5 && targets[3].hp == 5,
          "melee only strikes a nearby enemy ahead in the same lane");
    check(getPlayers()[0].attackAnimation > 0.0f && getPlayerProjectiles().empty(),
          "melee animates its weapon without emitting a projectile");
    updatePlayerCombat(0.1f);
    check(damageEvents == 1, "melee cooldown prevents per-frame damage");
    updatePlayerCombat(0.6f);
    check(damageEvents == 2, "melee repeats automatically after its cooldown");

    fresh();
    spawnPlayer(PLAYER_MELEE, 0);
    spawnPlayer(PLAYER_MELEE, 0);
    getPlayers()[0].x = getPlayers()[1].x = 0.0f;
    targets = {{41, 0, 0.1f, 1}, {99, 0, 0.115f, 2}};
    updatePlayerCombat(0.01f);
    check(targets.size() == 1 && targets[0].id == 99 && targets[0].hp == 1,
          "later attacks refresh enemy targets after a defeat erases a vector element");
}

static void testProjectiles()
{
    fresh();
    spawnPlayer(PLAYER_RANGED, 0);
    spawnPlayer(PLAYER_DEFENSE, 0);
    getPlayers()[0].x = -0.4f;
    getPlayers()[1].x = -0.1f;
    targets = {{2, 0, 0.4f, 5}, {3, 1, -0.1f, 5}, {1, 0, 0.1f, 1}};
    updatePlayerCombat(0.7f);
    check(targets.size() == 2 && targets[0].hp == 5 && targets[1].hp == 5 && damageEvents == 1,
          "spray hits the first same-lane enemy once, regardless of vector ordering");
    check(getPlayerProjectiles().empty(), "spray disappears immediately on impact");
    check(getPlayers()[0].hp == 5 && getPlayers()[1].hp == 7, "player projectiles never harm friendly units");

    fresh();
    connectPlayerCombatToEnemies({});
    spawnPlayer(PLAYER_RANGED, 0);
    updatePlayerCombat(0.01f);
    check(getPlayerProjectiles().size() == 1, "spray starts firing automatically even before enemy integration");
    float x = getPlayerProjectiles()[0].x;
    updatePlayerCombat(0.2f);
    check(std::fabs(getPlayerProjectiles()[0].x - x - SPRAY_PROJECTILE_SPEED * 0.2f) < 0.001f,
          "projectiles move using delta time");
    updatePlayerCombat(10.0f);
    check(getPlayerProjectiles().size() <= 2, "missed shots leave the battlefield and are removed");
    resetPlayerCombat();
    check(getPlayerProjectiles().empty(), "projectile reset clears every shot");
}

static void testDefense()
{
    fresh();
    spawnPlayer(PLAYER_MELEE, 0);
    spawnPlayer(PLAYER_DEFENSE, 0);
    spawnPlayer(PLAYER_DEFENSE, 1);
    getPlayers()[0].x = -0.4f;
    getPlayers()[1].x = -0.2f;
    getPlayers()[2].x = 0.1f;
    check(getPlayerDamageReceiver(0, 0.8f) == 1, "same-lane sheet intercepts an attack from ahead");
    damagePlayerFromEnemy(0, 2, 0.8f);
    check(getPlayers()[0].hp == 5 && getPlayers()[1].hp == 5, "sheet uses Member 1's HP and absorbs the whole hit");
    getPlayers()[0].x = 0.0f;
    damagePlayerFromEnemy(0, 1, 0.8f);
    check(getPlayers()[0].hp == 4, "an ally becomes vulnerable as soon as it passes the sheet");
    getPlayers()[0].x = -0.4f;
    damagePlayerFromEnemy(0, 1, -0.8f);
    check(getPlayers()[0].hp == 3, "a sheet does not intercept an attack arriving behind the ally");
    damagePlayerFromEnemy(0, 5, 0.8f);
    check(getPlayers().size() == 2 && getPlayerDamageReceiver(0, 0.8f) == 0,
          "a defeated sheet is removed and a sheet in another lane cannot protect the ally");
    damagePlayerFromEnemy(0, 1, 0.8f);
    check(getPlayers()[0].hp == 2, "protection ends immediately after sheet defeat");
    check(getPlayerDamageReceiver(-1, 0.8f) == -1 && getPlayerDamageReceiver(20, 0.8f) == -1,
          "invalid damage targets are ignored");
    updatePlayerCombat(2.0f);
    check(getPlayerProjectiles().empty(), "defense never fires projectiles");

    fresh();
    spawnPlayer(PLAYER_DEFENSE, 0);
    targets = {{1, 0, getPlayers()[0].x + 0.05f, 5}};
    updatePlayerCombat(3.0f);
    check(damageEvents == 0 && getPlayerProjectiles().empty(), "a sheet cannot melee or shoot a nearby enemy");
}

static void testTimingInputAndReset()
{
    for (int fps : {30, 60, 144}) {
        fresh();
        spawnPlayer(PLAYER_MELEE, 0);
        spawnPlayer(PLAYER_RANGED, 1);
        getPlayers()[0].x = getPlayers()[1].x = 0.0f;
        targets = {{1, 0, 0.1f, 100}, {2, 1, 0.2f, 100}};
        for (int frame = 0; frame < (int)(fps * 2.6f); frame++) updatePlayerCombat(1.0f / fps);
        check(targets[0].hp == 96 && targets[1].hp == 97, "attack cadence and impacts agree at 30/60/144 FPS");
    }

    fresh();
    spawnPlayer(PLAYER_RANGED, 0);
    clickLane(0);
    for (int key : {GLFW_KEY_EQUAL, GLFW_KEY_MINUS, GLFW_KEY_M, GLFW_KEY_N, GLFW_KEY_B,
                    GLFW_KEY_H, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5,
                    GLFW_KEY_C, GLFW_KEY_R}) handleUIKeyPress(nullptr, key, GLFW_PRESS);
    check(isGameplayRunning() && getPlayers().size() == 1 && getCollectibles() == 30 && getPlayers()[0].hp == 5,
          "removed test shortcuts cannot grant currency, units, damage, or state changes");
    handleUIKeyPress(nullptr, GLFW_KEY_UP, GLFW_PRESS);
    check(getPlayers()[0].pose == POSE_JUMPING && getCollectibles() == 30, "Up jumps without changing coins");
    handleUIKeyPress(nullptr, GLFW_KEY_DOWN, GLFW_PRESS);
    check(getPlayers()[0].pose == POSE_DUCKING && getCollectibles() == 30, "Down ducks without changing coins");
    updateGame(0.1f);
    selectPlayerCard(0);
    float x = getPlayerProjectiles()[0].x;
    float playerX = getPlayers()[0].x;
    float timer = getPlayers()[0].attackCooldown;
    pauseGame();
    updateGame(4.0f);
    updatePlayerCombat(4.0f);
    check(!deploySelectedPlayer(0), "pause blocks purchases");
    damagePlayerFromEnemy(0, 1, 0.9f);
    check(getPlayerProjectiles()[0].x == x && getPlayers()[0].x == playerX &&
          getPlayers()[0].attackCooldown == timer && getPlayers()[0].hp == 5,
          "pause freezes movement, shots, cooldowns and enemy damage");
    resumeGame();
    updatePlayerCombat(0.0f);
    updatePlayerCombat(-1.0f);
    updatePlayerCombat(std::numeric_limits<float>::infinity());
    check(getPlayerProjectiles()[0].x == x, "invalid combat delta times are ignored");

    for (int reset = 0; reset < 3; reset++) {
        if (reset == 0) restartGame();
        if (reset == 1) startNewGame();
        if (reset == 2) returnToMainMenu();
        check(getPlayers().empty() && getPlayerProjectiles().empty() && getSelectedCard() == -1 &&
              getCollectibles() == 30 && getEnvironment().getSelectedLanes().empty(),
              "every session reset clears players, shots, selection and purchasing state");
        setGameState(STATE_PLAYING);
        spawnPlayer(PLAYER_RANGED, 0);
        updatePlayerCombat(0.1f);
    }
    connectPlayerCombatToEnemies({});
}

int main()
{
    testPurchases();
    testMeleeAndEnemyRemoval();
    testProjectiles();
    testDefense();
    testTimingInputAndReset();
    std::cout << "Player ability tests passed: purchasing, attacks, projectiles, protection, timing and reset.\n";
}
