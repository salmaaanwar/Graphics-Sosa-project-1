# SOSA Graphics — Renderer Guide

## Files

- `main.cpp` — window, shaders, geometry, projection, and main loop.
- `Renderer.h`, `Renderer.cpp` — shapes, transformations, and mouse conversion.
- `TextRenderer.h`, `TextRenderer.cpp` — text and number drawing.
- `ExampleScene.h`, `ExampleScene.cpp` — working examples.
- `Game.h`, `Game.cpp` — shared states, timing, pause, restart and result transitions.
- `Cutscene.h`, `Cutscene.cpp` — the 16-second animated kitchen introduction.
- `Deployment.h`, `Deployment.cpp` — card selection, purchasing, and shared coins.
- `PlayerCombat.h`, `PlayerCombat.cpp` — player attacks, spray projectiles, and Defense protection.
- `CMakeLists.txt` — source files and library configuration.

For Member 4's implementation and a lab-style explanation of the animation,
timing and integration changes, see [docs/MEMBER4.md](docs/MEMBER4.md).

## Build and Run

Run these commands from the project folder. Each teammate builds locally.

### macOS

```bash
cmake -S . -B build
cmake --build build && ./build/OpenGLStarter
```

### Windows — Visual Studio

```powershell
cmake -S . -B build
cmake --build build --config Debug
.\build\Debug\OpenGLStarter.exe
```

For MinGW or Ninja builds, the executable is usually:

```powershell
.\build\OpenGLStarter.exe
```

Save and rebuild after changing code. Launch only after a successful build.

For the full test commands, results, fixed timing bugs, and remaining gameplay
gaps, see [docs/TESTING.md](docs/TESTING.md). Optional CMake test targets cover
game flow, player abilities, movement/input stress, and hidden-window rendering.

## Using the Renderer

Include the headers your module needs:

```cpp
#include "Renderer.h"
#include "TextRenderer.h"
```

Set a color before drawing:

```cpp
glUniform3f(colorLocation, 1.0f, 0.0f, 0.0f);
drawRectangle(transformLocation, 0.0f, 0.0f, 0.4f, 0.2f);
```

### Available Functions

| Function | Parameters / behavior |
|---|---|
| `drawRectangle` | Center, full width/height, optional rotation in degrees |
| `drawCircle` | Center, radius, and geometry range |
| `drawTriangle` | Local-origin position, width/height, geometry start, optional rotation |
| `drawDigit`, `drawNumber`, `drawText` | Top-left position and cell size |
| `mouseToWorld` | Cursor position converted to world coordinates; returns success |

Text supports uppercase letters, digits, and spaces. Numbers must be nonnegative.

Follow `ExampleScene.cpp` for complete calls. Pass the required uniform locations and geometry indices from `main.cpp` into your module.

### Coordinates and Drawing Order

- Screen center is `(0, 0)`.
- Positive X goes right; positive Y goes up.
- The central square from −1 to +1 stays visible when resizing.
- Draw backgrounds first, characters next, and HUD/text last.
- Later shapes cover earlier shapes.

## Module Organization

| Part | Suggested files |
|---|---|
| Rendering | `Renderer.h/.cpp`, `TextRenderer.h/.cpp` |
| Environment and lanes | `Environment.h/.cpp` |
| Menus and HUD | `UI.h/.cpp` |
| Cutscene and game flow | `Cutscene.h/.cpp`, `Game.h/.cpp` |

Declare functions in headers and implement them in `.cpp` files.

Add each new `.cpp` to `add_executable(...)` in `CMakeLists.txt`.

Use the existing window and render loop. In `main.cpp`, call scene drawing functions after `glBindVertexArray(VAO)` and before `glfwSwapBuffers(window)`.

## Examples

Find this immediately before the render loop in `main.cpp`:

```cpp
const bool showExamples = true;
```

Set it to `false` to hide the demonstrations when integrating actual scenes.

Coordinate changes to shared files and function parameters with the team.

---

## Menus & HUD Module (`UI.h` / `UI.cpp`) — Member 3

Implemented using the existing `Renderer` and `TextRenderer` utilities with composite multi-primitive shapes matching our **Kitchen Countertop (Cleaning Detergents vs. Insects)** theme.

### 1. Shared Game States & Functions (`UI.h`)

State definitions and transition functions now live in `Game.h` / `Game.cpp`.
`UI.h` includes `Game.h`, so the same names remain available to UI callers.

```cpp
const int STATE_MAIN_MENU = 0;
const int STATE_CUTSCENE  = 1;
const int STATE_PLAYING   = 2;
const int STATE_PAUSED    = 3;
const int STATE_WON       = 4;
const int STATE_LOST      = 5;

const int MELEE_COST   = 10; // Fly Swatter (Melee)
const int RANGED_COST  = 60; // Spray Bottle (2x starting budget)
const int DEFENSE_COST = 90; // Sticky Sheet (3x starting budget)

int getGameState();
void setGameState(int state);
int getCollectibles();
void setCollectibles(int amount);
int getSelectedCard();
void resetDeployment();
```

### 2. Implemented Menus

- **Main Menu (`drawMainMenu`):**
  - Displays the kitchen-themed title (`KITCHEN DEFENSE — DETERGENTS VS INSECTS`), composite character icons, and interactive hover/click buttons:
    - **`START GAME`** — resets the session, plays the animated intro (`STATE_CUTSCENE`), then automatically enters `STATE_PLAYING`.
    - **`EXIT GAME`** — closes the application window.
- **Pause Menu (`drawPauseMenu`):**
  - Triggered during gameplay with **`ESC`** (or by clicking the top-right **`PAUSE`** HUD button).
  - Renders over the paused scene with interactive hover/click buttons:
    - **`CONTINUE`** — resumes `STATE_PLAYING`.
    - **`RESTART`** — resets collectibles/health and restarts `STATE_PLAYING`.
    - **`MAIN MENU`** — resets state and returns to `STATE_MAIN_MENU`.
- **Win / Lose Menu (`drawWinLoseMenu`):**
  - Shown when `getGameState()` is `STATE_WON` (`YOU WIN — KITCHEN IS CLEAN`) or `STATE_LOST` (`YOU LOSE — INSECTS REACHED THE FOOD`).
  - Interactive hover/click buttons for **`RESTART`** and **`MAIN MENU`**.

### 3. Gameplay Top HUD & Player Cards (`drawHUD`)

- **Collectibles Counter:** Drawn at the top-right with a composite soapy coin icon (`drawCollectibleIcon`) and live numeric counter (`drawNumber`), starting at `30` coins.
- **3 Player Cards (`SWATTER` = 10, `SPRAY` = 60, `STICKY` = 90):**
  - Each card features a multi-primitive composite icon:
    - `drawSwatterIcon` — Melee Fly Swatter
    - `drawSprayBottleIcon` — Long Range Spray Cleaning Bottle
    - `drawStickySheetIcon` — Stationary Defense Sticky Sheet Trap
  - **Locked vs. Unlocked Behavior (`collectibles >= cost`):**
    - At `30` starting coins, only **`SWATTER` (10)** is unlocked; **`SPRAY` (60)** and **`STICKY` (90)** start locked.
    - **Locked cards (`collectibles < cost`):** Rendered completely in **grayscale** and ignore mouse hover and click events.
    - **Unlocked cards (`collectibles >= cost`):** Rendered in **full color**, highlight on mouse hover, and can be clicked to select/deselect for lane placement.
    - **Dynamic Unlock / Re-lock:** Cards automatically unlock when `collectibles` reaches `60` or `90`, and automatically return to their locked grayscale state whenever `collectibles` drops below their cost.

### 4. Reusable Unit Health Display (`drawHealthBar`)

```cpp
void drawHealthBar(
    GLint transformLocation,
    GLint colorLocation,
    float x,
    float y,
    int currentHP,
    int maxHP
);
```

- Draws a segmented health bar + numeric HP count above/below any player or enemy at `(x, y)`.
- Automatically scales the fill width by `currentHP / maxHP` and shifts color from **green** (high HP) to **yellow** (medium HP) to **red** (low HP).

### 5. Gameplay Controls

| Input | Action |
|---|---|
| **Mouse Hover & Left Click** | Menu buttons, HUD `PAUSE`, and affordable player cards |
| **Card, then Left Click on Lane** | Purchase exactly one unit at that lane's left edge |
| **Left Click on Lane without an armed card** | Select/deselect a lane; multiple lanes may be selected |
| **W / Up** | Jump with eligible players in every selected lane |
| **S / Down** | Duck with eligible players in every selected lane |
| **Escape** | Pause/resume gameplay |

Defense ignores jump/duck. Pause and results freeze gameplay. Start plays the
intro; Restart begins a fresh game directly. Currency, free-spawn, damage and
forced-state development shortcuts have been removed.

## Member 2 Environment and Lane System

Added `Environment.h` and `Environment.cpp` for a four-lane countertop battlefield, selectable lane highlights, a composite food home base and kitchen details. Decorative placeholder units are disabled for gameplay. Lane geometry, colors, and placeholder mode are named constants near the top of `Environment.cpp`.

`main.cpp` routes gameplay-area clicks to Environment and keeps HUD/menu clicks with UI. It renders Environment before the existing HUD and pause/end overlays, while the intro uses `drawCutscene`. The shared game controller owns the Environment instance, exposed through `getEnvironment()`, advances it only while Playing, and resets it on Start, Restart and Main Menu. Continue preserves lane selection and animation time. `CMakeLists.txt` registers `Environment.cpp` for the application and optional game-flow tests. Gameplay-area clicks select lanes or confirm an armed card purchase before HUD/menu handling.

Public API:

```cpp
void update(float dt);
void render(GLint transformLocation, GLint colorLocation, int circleVertexStart, int circleVertexCount, int triangleVertexStart);
void handleMouseMove(float mouseX, float mouseY);
void handleMouseButton(int button, int action, float mouseX, float mouseY);
int getLaneCount() const;
float getLaneCenterY(int i) const;
float getLaneTop(int i) const;
float getLaneBottom(int i) const;
float getLaneStartX() const;
float getLaneEndX() const;
float getHomeBaseRightEdgeX() const;
int getLaneAtPoint(float x, float y) const;
bool isLaneSelected(int i) const;
std::vector<int> getSelectedLanes() const;
void clearSelection();
void reset();
```

Lane clicks now purchase one unit when a card is armed; otherwise they toggle
selection. Player attacks, projectiles, and Defense routing are implemented in
`PlayerCombat`. Enemy models, enemy movement/attacks, enemy HP, waves, and drops
still need their owning members' gameplay modules and the enemy adapter connection.


## Member 1 — Player Models, Movement, and Health (`Player.h` / `Player.cpp`)

Added `Player.h` and `Player.cpp` for shared player instances, detailed gameplay models (adapted from the Swatter, Spray Bottle, and Sticky Sheet icons), automatic rightward movement for Melee and Long Range, stationary Defense, jumping (`W`/`Up`) and ducking (`S`/`Down`) in selected lanes, health initialization (`5`, `5`, `7`), progressive redness as health decreases, `drawHealthBar` connection, and removal on defeat.

Shared API in `Player.h`:

```cpp
const int PLAYER_MELEE   = 0;
const int PLAYER_RANGED  = 1;
const int PLAYER_DEFENSE = 2;

const int MELEE_MAX_HP   = 5;
const int RANGED_MAX_HP  = 5;
const int DEFENSE_MAX_HP = 7;

const int POSE_NORMAL  = 0;
const int POSE_JUMPING = 1;
const int POSE_DUCKING = 2;

const float PLAYER_HALF_WIDTH  = 0.045f;
const float PLAYER_HALF_HEIGHT = 0.075f;

int spawnPlayer(int type, int lane);
void updatePlayers(float dt);
void handlePlayerKeyPress(int key, int action);
void damagePlayer(int index, int damage);
void setPlayerBlocked(int index, bool blocked);
void clearAllPlayerBlocks();
std::vector<Player>& getPlayers();
void resetPlayers();
void drawPlayers(GLint transformLocation, GLint colorLocation, int circleVertexStart, int circleVertexCount, int triangleVertexStart);
```


## Member 2 - Player Attacks, Defense, and Deployment

`Deployment.h/.cpp` connects the existing cards to actual purchases.
`PlayerCombat.h/.cpp` implements automatic one-damage swatter swings and cleaning
spray projectiles, attack animations, same-lane swept collision, and sticky-sheet
protection through Member 1's HP system. Prices are 10/60/90, with 30 starting
coins; 60 and 90 use 2x/3x the starting budget as the chosen unlock baseline.

See [docs/MEMBER2.md](docs/MEMBER2.md) for controls, the explicit purchasing rule,
lab-style implementation notes, enemy integration functions for Members 3/4,
reset behavior, and build/test instructions. The enemy adapter has no production
provider yet; its test-only provider verifies damage and removal without adding
another member's enemy or health implementation.
