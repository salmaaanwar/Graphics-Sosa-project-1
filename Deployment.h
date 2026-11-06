#pragma once

// The starting budget buys three melee units. The other cards are priced
// at two and three times the melee cost.
const int STARTING_COLLECTIBLES = 30;
const int MELEE_COST = 10;
const int RANGED_COST = 2 * MELEE_COST;
const int DEFENSE_COST = 3 * MELEE_COST;

int getCollectibles();
void setCollectibles(int amount);
void collectEnemyDrop(int value);
int getRemainingUnlockDrops(int type);
int getPlayerCost(int type);
bool isCardUnlocked(int type);
int getSelectedCard();
void selectPlayerCard(int type);

// Places the selected card in a lane. Invalid placements spend nothing.
bool deploySelectedPlayer(int lane);
void resetDeployment();
