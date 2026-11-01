#include "Deployment.h"
#include "Environment.h"
#include "Game.h"
#include "Player.h"

static int collectibles = STARTING_COLLECTIBLES;
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

bool isCardUnlocked(int type)
{
    int cost = getPlayerCost(type);
    return cost > 0 && collectibles >= cost;
}

void setCollectibles(int amount)
{
    collectibles = amount < 0 ? 0 : amount;
    if (!isCardUnlocked(selectedCard)) selectedCard = -1;
}

void selectPlayerCard(int type)
{
    if (!isGameplayRunning() || !isCardUnlocked(type)) return;
    selectedCard = selectedCard == type ? -1 : type;
}

bool deploySelectedPlayer(int lane)
{
    if (!isGameplayRunning() || !isCardUnlocked(selectedCard)) return false;
    if (lane < 0 || lane >= getEnvironment().getLaneCount()) return false;

    // Friendly units may overlap and pass one another. A second stationary sheet
    // at the same deployment position would hide the first, so reject that case.
    if (selectedCard == PLAYER_DEFENSE) {
        const std::vector<Player>& players = getPlayers();
        for (int i = 0; i < (int)players.size(); i++) {
            if (players[i].lane == lane && players[i].type == PLAYER_DEFENSE && players[i].hp > 0)
                return false;
        }
    }

    int cost = getPlayerCost(selectedCard);
    if (spawnPlayer(selectedCard, lane) < 0) return false;
    setCollectibles(collectibles - cost);
    selectedCard = -1;
    return true;
}

void resetDeployment()
{
    collectibles = STARTING_COLLECTIBLES;
    selectedCard = -1;
}
