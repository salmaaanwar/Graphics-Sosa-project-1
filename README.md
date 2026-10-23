# SOSA Graphics — Renderer Guide

## Files

- `main.cpp` — window, shaders, geometry, projection, and main loop.
- `Renderer.h`, `Renderer.cpp` — shapes, transformations, and mouse conversion.
- `TextRenderer.h`, `TextRenderer.cpp` — text and number drawing.
- `ExampleScene.h`, `ExampleScene.cpp` — working examples.
- `CMakeLists.txt` — source files and library configuration.

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

```cpp
const int STATE_MAIN_MENU = 0;
const int STATE_CUTSCENE  = 1;
const int STATE_PLAYING   = 2;
const int STATE_PAUSED    = 3;
const int STATE_WON       = 4;
const int STATE_LOST      = 5;

const int MELEE_COST   = 10; // Fly Swatter (Melee)
const int RANGED_COST  = 40; // Spray Bottle (Long Range)
const int DEFENSE_COST = 60; // Sticky Sheet (Defense)

int getGameState();
void setGameState(int state);
int getCollectibles();
void setCollectibles(int amount);
int getSelectedCard();
void resetGamePlaceholders();
```

### 2. Implemented Menus

- **Main Menu (`drawMainMenu`):**
  - Displays the kitchen-themed title (`KITCHEN DEFENSE — DETERGENTS VS INSECTS`), composite character icons, and interactive hover/click buttons:
    - **`START GAME`** — resets placeholders and starts gameplay (`STATE_PLAYING`).
    - **`EXIT GAME`** — closes the application window.
- **Pause Menu (`drawPauseMenu`):**
  - Triggered during gameplay with **`ESC`** (or by clicking the top-right **`PAUSE`** HUD button).
  - Renders over the paused scene with interactive hover/click buttons:
    - **`CONTINUE`** — resumes `STATE_PLAYING`.
    - **`RESTART`** — resets collectibles/health and restarts `STATE_PLAYING`.
    - **`MAIN MENU`** — resets state and returns to `STATE_MAIN_MENU`.
- **Win / Lose Menu (`drawWinLoseMenu`):**
  - Shown when `currentGameState` is `STATE_WON` (`YOU WIN — KITCHEN IS CLEAN`) or `STATE_LOST` (`YOU LOSE — INSECTS REACHED THE FOOD`).
  - Interactive hover/click buttons for **`RESTART`** and **`MAIN MENU`**.

### 3. Gameplay Top HUD & Player Cards (`drawHUD`)

- **Collectibles Counter:** Drawn at the top-right with a composite soapy coin icon (`drawCollectibleIcon`) and live numeric counter (`drawNumber`), starting at `30` coins.
- **3 Player Cards (`SWATTER` = 10, `SPRAY` = 40, `STICKY` = 60):**
  - Each card features a multi-primitive composite icon:
    - `drawSwatterIcon` — Melee Fly Swatter
    - `drawSprayBottleIcon` — Long Range Spray Cleaning Bottle
    - `drawStickySheetIcon` — Stationary Defense Sticky Sheet Trap
  - **Locked vs. Unlocked Behavior (`collectibles >= cost`):**
    - At `30` starting coins, only **`SWATTER` (10)** is unlocked; **`SPRAY` (40)** and **`STICKY` (60)** start locked.
    - **Locked cards (`collectibles < cost`):** Rendered completely in **grayscale** and ignore mouse hover and click events.
    - **Unlocked cards (`collectibles >= cost`):** Rendered in **full color**, highlight on mouse hover, and can be clicked to select/deselect for lane placement.
    - **Dynamic Unlock / Re-lock:** Cards automatically unlock when `collectibles` reaches `40` or `60`, and automatically return to their locked grayscale state whenever `collectibles` drops below their cost.

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

### 5. Standalone Test Controls

| Input | Action |
|---|---|
| **Mouse Hover & Left Click** | Interact with menu buttons, HUD `PAUSE` button, and unlocked Player Cards |
| **Left Click on Lane (with Card selected)** | Simulates placing that unit by spending its coin cost (`10`, `40`, or `60`) |
| **`ESC`** | Toggle between `STATE_PLAYING` and `STATE_PAUSED` |
| **`UP` Arrow** or **`+` (`=`)** | Add `+5` coins (test unlocking `SPRAY` at `40` and `STICKY` at `60`) |
| **`DOWN` Arrow** or **`-`** | Subtract `-5` coins (test automatic grayscale re-locking below `60`, `40`, and `10`) |
| **`H`** | Deal `1` damage to all placeholder units on the lanes to test `drawHealthBar` |
| **`R`** | Reset placeholder coins (`30`) and unit health values |
| **`1` / `2` / `3` / `4` / `5` / `C`** | Jump directly to Main Menu (`1`), Playing + HUD (`2`), Paused (`3`), Won (`4`), Lost (`5`), or Cutscene (`C`) |
## Member 2 Environment and Lane System

Added `Environment.h` and `Environment.cpp` for a four-lane countertop battlefield, selectable lane highlights, a composite food home base, kitchen details, and optional moving placeholder units with health bars. Lane geometry, colors, and placeholder mode are named constants near the top of `Environment.cpp`.

`main.cpp` routes gameplay-area clicks to Environment and keeps HUD/menu clicks with UI. It renders Environment before the existing HUD and pause/end overlays. `CMakeLists.txt` registers `Environment.cpp`. The UI placeholder lane click behavior is avoided by consuming gameplay-area clicks before `handleUIMouseClick`; `UI.cpp` was not changed.

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

Build verification was unavailable in this environment because `cmake` is not installed or on PATH. No runtime visual verification was performed.
