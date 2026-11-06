#include <algorithm>
#include <cmath>

#include "Deployment.h"
#include "Environment.h"
#include "Game.h"
#include "Player.h"


static int collectibles = STARTING_COLLECTIBLES;
static int enemyDropsCollected = 0;
static int selectedCard = -1;

int getCollectibles() { return collectibles; }
int getSelectedCard() { return selectedCard; }

int getPlayerCost(int type)
{
    if (type == PLAYER_MELEE) return MELEE_COST;
    if (type == PLAYER_RANGED) return RANGED_COST;
    if (type == PLAYER_DEFENSE) return DEFENSE_COST;
    return -1;
}

int getRemainingUnlockDrops(int type)
{
    int required = 0;
    if (type == PLAYER_RANGED) required = 2;
    if (type == PLAYER_DEFENSE) required = 3;

    int remaining = required - enemyDropsCollected;
    return remaining > 0 ? remaining : 0;
}

bool isCardUnlocked(int type)
{
    int cost = getPlayerCost(type);
    return cost > 0 && collectibles >= cost && getRemainingUnlockDrops(type) == 0;
}

void setCollectibles(int amount)
{
    collectibles = amount < 0 ? 0 : amount;
    if (!isCardUnlocked(selectedCard)) selectedCard = -1;
}

void collectEnemyDrop(int value)
{
    ++enemyDropsCollected;
    setCollectibles(collectibles + value);
}

void selectPlayerCard(int type)
{
    if (!isGameplayRunning() || !isCardUnlocked(type)) return;

    const auto lanes = getEnvironment().getSelectedLanes();
    if (lanes.empty()) return;

    for (int lane : lanes) {
        if (!isCardUnlocked(type)) break;

        // Deploy into a lane; defense uses its remembered click position.
        // Failed placements spend nothing.
        selectedCard = type;
        deploySelectedPlayer(lane);
    }

    selectedCard = -1;
}

bool deploySelectedPlayer(int lane)
{
    if (!isGameplayRunning() || !isCardUnlocked(selectedCard)) {
        return false;
    }

    const Environment& env = getEnvironment();

    if (lane < 0 || lane >= env.getLaneCount()) {
        return false;
    }

    float x = env.getPlacementX(lane);
    if (!std::isfinite(x)) return false;

    // Keep the defense hitbox inside the lane.
    float left = env.getLaneStartX() + PLAYER_HALF_WIDTH;
    float right = env.getLaneEndX() - PLAYER_HALF_WIDTH;

    if (x < left) x = left;
    if (x > right) x = right;

    // Multiple defenses may share a lane if they do not overlap.
    if (selectedCard == PLAYER_DEFENSE) {
        for (const Player& player : getPlayers()) {
            if (player.lane == lane &&
                player.type == PLAYER_DEFENSE &&
                player.hp > 0 &&
                std::fabs(player.x - x) < 2.0f * PLAYER_HALF_WIDTH) {
                return false;
            }
        }
    }

    int type = selectedCard;
    int cost = getPlayerCost(type);

    int index = spawnPlayer(type, lane);
    if (index < 0) return false;

    if (type == PLAYER_DEFENSE) {
        getPlayers()[index].x = x;
    }

    setCollectibles(collectibles - cost);
    selectedCard = -1;

    return true;
}

void resetDeployment()
{
    collectibles = STARTING_COLLECTIBLES;
    enemyDropsCollected = 0;
    selectedCard = -1;
}
