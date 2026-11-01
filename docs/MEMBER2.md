# Member 2: Player attacks, Defense, and deployment

## What is implemented

`Deployment.h/.cpp` owns card selection and the shared coin balance. Select an
available HUD card, then click one lane to purchase exactly one unit at Member 1's
leftmost lane position. The click selects that lane for jump/duck commands. With
no armed card, lane clicks retain their existing select/deselect behavior,
including multiple selected lanes. Selecting a lane before a card is also fine:
the next lane click confirms which single lane receives the purchase.

Invalid lanes, insufficient coins, menus, pause, and duplicate stationary sheets
in the same lane spend nothing. A successful purchase clears the armed card.
Friendly moving units may overlap, including passing through a sticky sheet.
The mouse press/release latch prevents repeated press events from buying twice.

The starting balance is 30, enough for three melee units at 10 each. The brief's
2x/3x wording has an ambiguous baseline: multiplying the melee price of 10 would
make every card available with the starting 30 coins. This implementation uses
the **starting budget of 30** as the explicit baseline, giving spray a price and
availability threshold of **60**, and sticky sheet **90**. Current balance controls
availability, so cards return to grayscale after spending below their price.
There is no separate permanent unlock state. These choices are centralized in
`Deployment.h` and can be changed together if the team agrees on another reading.

`PlayerCombat.h/.cpp` supplies:

- Automatic mosquito-bat/swatter swings every 0.75 seconds. Each swing damages
  only the closest overlapping or nearby enemy ahead in the same lane, by 1 HP.
- Automatic cleaning-bottle shots every 1 second, with a synchronized trigger
  animation and nozzle mist. Each moving droplet deals 1 HP to the first enemy
  it reaches, disappears on impact, and never damages friendly players.
- Swept horizontal projectile collision, so a projectile cannot skip a narrow
  target between frames. Missed projectiles are removed at the lane's right edge.
- Sticky-sheet protection: enemy hits on an ally behind a living sheet in its
  lane go to the frontmost sheet between the ally and the attack's origin. The
  sheet uses Member 1's existing 7 HP, red damage tint, and defeat/removal code.
  A green underline marks protected allies. Protection ends when an ally passes
  the sheet or the sheet is removed. Defense does not attack or emit projectiles.

Free starter units and moving decorative enemies are disabled. The old currency,
free-spawn, damage, restart, and forced-state test keys have been removed. W/Up
and S/Down only control player poses; Escape and the existing menu buttons control
pause/resume and session resets. The kitchen primitives and existing cards,
hover styling, grayscale appearance, health bars, and coin counter are reused.

## How this follows the labs

Like Lab 3, the implementation uses structs, vectors, ordinary loops, GLFW press
events, and coordinate checks. Existing renderer helpers draw rectangles, circles,
and triangles using the shared VAO/VBO, GLM transforms, and color uniforms.
Weapon motion uses a sine value to offset the swatter head and rotate the trigger.
Like Lab 4, movement and cooldowns use elapsed seconds rather than frame counts.
Combat divides a long frame into short updates for repeatable firing and impacts.
No physics engine, texture assets, or additional rendering library is introduced.

## Enemy integration for Members 3 and 4

There is no production enemy gameplay/health module in this checkout. This module
does not invent a competing enemy store, health system, or wave implementation.
Attacks animate and spray shots move now; combat damage requires the enemy team
to register the following three ordinary function pointers once during setup:

```cpp
int countEnemies();
bool readEnemyTarget(int index, PlayerAttackTarget& target);
void damageEnemyById(int id, int amount);

connectPlayerCombatToEnemies({countEnemies, readEnemyTarget, damageEnemyById});
```

`readEnemyTarget` fills `{id, lane, x, halfWidth}` from the enemy's current state.
IDs are nonnegative, unique during an enemy's lifetime, and stable when vector
elements are erased. Skip defeated enemies by returning false. `damageEnemyById`
must use Member 4's health/death/drop function. Targets are read again for every
attack, so immediate enemy removal during damage does not invalidate a cached
target. The callbacks must only damage enemies, not mutate the player vector or
change game state; Member 4 should evaluate wave completion after combat updates.
The registration survives a session reset; the enemy team resets its own enemies.
Pass `{}` to disconnect when the provider is no longer available.

For incoming attacks, resolve the live player index immediately before each hit:

```cpp
int receiver = getPlayerDamageReceiver(playerIndex, enemyOrProjectileX);
if (receiver >= 0) {
    // Inspect getPlayers()[receiver].pose for the enemy attack's dodge rule.
    // A sticky sheet cannot jump or duck and therefore absorbs the attack.
    // If the hit was not dodged:
    damagePlayer(receiver, enemyDamage);
}
```

`damagePlayerFromEnemy(index, damage, attackerX)` combines damage routing and
damage application for an already confirmed hit; it does not apply jump/duck
rules. In particular, resolve the receiver before checking a ranged dodge so a
ducking protected ally cannot make its blocking sheet evade the incoming attack.
Do not retain player indices after another player is removed. The enemy team's
projectile code must consume a blocked projectile once and apply no overflow
damage when the sheet is defeated.

Member 3 still owns encounter stopping/resuming, enemy attacks, enemy projectile
height/dodging, and the home-base collision. Existing `setPlayerBlocked` and
`clearAllPlayerBlocks` remain available. Member 4 still owns enemy HP, waves,
collectible drops and collision collection, and actual Win/Lose conditions.
Award coins through `setCollectibles(getCollectibles() + dropValue)`; it immediately
updates card availability and clears a selection if it becomes unaffordable.

## Timing, drawing, and reset

The game controller updates player movement and combat in shared short time
steps, only in Playing, so a long frame does not shift the origin of a shot.
`updatePlayerCombat` also guards against calls during menus, pause, and results.
Draw projectiles after players and before the HUD; paused/result scenes retain
their last positions. `resetPlayerCombat()` clears projectiles and attack timers.
`resetDeployment()` restores 30 coins and clears the selected card. Start, Restart,
and Main Menu call both resets alongside Member 1's player reset and the existing
environment/cutscene resets.

## Verification

Build the game and optional headless checks with CMake, Ninja, and GCC on PATH:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DBUILD_GAME_FLOW_TESTS=ON -DBUILD_PLAYER_ABILITY_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

`PlayerAbilitiesTests` uses a test-only enemy provider. It checks purchasing and
placement failures, repeated mouse presses, affordability thresholds, melee
range/cooldowns, enemy erasure during attacks, projectile ordering/impact/removal,
no friendly fire, shield durability and loss of protection, 30/60/144 FPS attack
cadence, removed development keys, pause, and every session reset. `GameFlowTests`
continues to check the intro, menus, pause, results, and lane reset integration.

The game and both test targets built with GCC/CMake and both test suites passed.
A hidden OpenGL window also rendered all three models, a swatter swing, trigger
mist, a moving spray projectile, the protection underline, and the HUD without
shader or OpenGL errors. The local `build-member2/render-check.png` capture is an
ignored verification artifact, not a game asset.
