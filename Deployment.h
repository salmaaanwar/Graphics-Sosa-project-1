#pragma once

// The initially unlocked melee card costs 10; the starting budget buys three.
// Use that 30-coin starting budget as the explicit 2x/3x unlock baseline.
const int STARTING_COLLECTIBLES = 30;
const int MELEE_COST = 10;
const int RANGED_COST = 2 * STARTING_COLLECTIBLES;
const int DEFENSE_COST = 3 * STARTING_COLLECTIBLES;

int getCollectibles();
void setCollectibles(int amount);
int getPlayerCost(int type);
bool isCardUnlocked(int type);
int getSelectedCard();
void selectPlayerCard(int type);

// One selected card + one valid lane click = one purchase. Failures spend nothing.
bool deploySelectedPlayer(int lane);
void resetDeployment();
