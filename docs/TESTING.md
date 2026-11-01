# Repository test report

Tested on November 1, 2026 against the current local working tree on `main`
(base commit `15d483a`, including the uncommitted Member 2 implementation).

## Result

**8/8 CTest cases pass in both Debug and Release.** The implemented modules build,
their tested behavior passes, and their rendering works on the tested Windows
machine. This does **not** establish a complete playable game: the production
enemy, wave, collectible, and automatic victory/defeat systems are still absent.

| Check | Result |
| --- | --- |
| Fresh GCC/Ninja Debug build | Pass, with `_GLIBCXX_DEBUG` and `_GLIBCXX_ASSERTIONS` enabled |
| Fresh GCC/Ninja Release build | Pass |
| All first-party C++ and test files under `-Wall -Wextra -Wpedantic -Werror` | Pass, zero warnings |
| `install_tools.py` Python syntax | Pass; installer was not executed |
| CTest suites in each build | 8 passed, 0 failed |
| Deterministic mixed-input stress test | 10,000 sequences, including 1,242 successful purchases per run |
| Hidden-window OpenGL check | 70 rendered frames per configuration; no shader, drawing, or readback errors |
| Pause rendering | Pixel-identical before/after a simulated 30-second paused update at all five sizes |
| Patch whitespace check | `git diff --check` passes |

Toolchain: Windows, GCC 15.2.0, CMake/Ninja from the existing local tool bundle.
Graphics: NVIDIA GeForce RTX 3060 Laptop GPU, OpenGL `4.0.0 NVIDIA 616.56`.
Build and test evidence is in ignored directories `build-qa-debug`,
`build-qa-release`, and `build-qa`.

## Coverage

The existing `PlayerAbilities` and `GameFlow` suites cover purchasing, card
availability, one-unit-per-purchase behavior, invalid placements, same-lane
attacks, one-damage hits, projectile ordering and removal, friendly-fire
prevention, shield durability/routing, removed development shortcuts, cutscene
timing, menu transitions, pause, restart, and reset.

New `RepositoryTests` scenarios cover:

- **movement:** melee/ranged movement at 30/60/144 FPS, stationary Defense,
  encounter stop/resume flags, selected-lane poses, jump expiry, right boundary,
  damage, and removal without affecting the neighboring player.
- **stress:** exact lane edges and separator gaps, followed by 10,000 seeded
  mixed selections, purchases, invalid requests, pauses, updates, and resets.
  Each transaction checks both player count and coin conservation.
- **invalid-time:** zero, negative, NaN, and infinite time inputs during gameplay
  and the intro.
- **long-frame:** the same moving shooter simulated as sixty 1/120-second frames
  and one 0.5-second frame; both player and projectile positions must agree.
- **frozen-states:** player movement, projectiles, pose commands, and purchasing
  remain inactive in Main Menu, Cutscene, Pause, Won, and Lost.

New `RenderSmokeTests` creates a hidden OpenGL window and compiles the production
shaders. It exercises the application's real mouse/key callbacks with injected
world coordinates, renders intro samples and all menu/gameplay states, purchases
all three player types, checks pause, restarts from results, and checks Exit.
It tests 800x600, 1280x720, 600x800, 320x240, and 1920x1080 window sizes, including
viewport resizing and the zero-sized framebuffer guard. This tests callback
routing; it does not test physical mouse events or OS cursor-to-world mapping.

Visual review inspected the generated montage of menu, intro, gameplay, pause,
Win/Lose, portrait, and widescreen captures. Captures are available locally under
`build-qa-debug/render-captures`; the montage is `build-qa/render-overview.png`.

## Reproduced failures and fixes

1. **Long frames advanced the shooter before simulating all its shots.** Before
   the fix, the regression produced projectile X `-0.137834` at 120 FPS versus
   `-0.0690002` after one 0.5-second frame. `Game.cpp` now advances movement and
   combat together in steps of at most 1/120 second. The results now agree:
   `-0.137834` versus `-0.137833`, within the test's 0.001 tolerance.
2. **NaN/infinite elapsed time corrupted or skipped game state.** Four assertions
   failed before the fix. `Game.cpp`, `Player.cpp`, and `Cutscene.cpp` now reject
   non-finite elapsed times alongside nonpositive values.
3. **Four unused callback parameter warnings.** Unused parameter names were
   removed from the GLFW callbacks; callback signatures and behavior are unchanged.
4. **Advertised CMake minimum was too low.** The root required 3.10 while bundled
   GLFW requires 3.16. The root now declares 3.16.

Both timing regressions were run and observed failing before their fixes. All
eight final test cases passed in both build configurations after the fixes.

## Remaining gaps and limits

- **Complete gameplay is not available.** `connectPlayerCombatToEnemies` has no
  production provider. Enemy models/movement/attacks, enemy HP, encounter
  coordination, waves, collectible drops/pickup, and automatic Win/Lose are not
  integrated. Enemy damage tests use a test-only provider; result-menu tests
  trigger results explicitly. No full three-wave playthrough was possible.
- **The requested kitchen prop set is incomplete.** The environment currently
  draws a sink, cabinet/window, toaster, cutting board, mug, and shakers. The
  requested pots/pans, utensils, stove, and fridge are not present. The HUD also
  covers much of the upper kitchen strip, including the top of the faucet.
- **Defense placement needs gameplay review.** All types spawn at the same X.
  A newly deployed moving ally immediately advances past a stationary sheet's
  center, so useful behind-the-sheet protection is very brief in the current
  deployment layout. Routing tests verify protection when the ally is actually
  behind the sheet; they do not resolve this placement/design constraint.
- **Unlock baseline remains an explicit interpretation.** Prices are 10/60/90
  using the 30-coin starting budget for the 2x/3x multipliers, as documented in
  `MEMBER2.md`. Tests establish that implementation's behavior, not rubric approval.
- Testing was on Windows/GCC with one NVIDIA driver. MSVC, macOS/Linux, other
  GPUs, real high-DPI mouse interaction, and screen-reader accessibility were not
  exercised. Checked C++ containers are enabled in Debug; no ASan/UBSan or leak
  sanitizer run was performed.

## Reproduce

With CMake, Ninja, GCC, and G++ on PATH, from the repository root:

```powershell
cmake -S . -B build-test-debug -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug "-DCMAKE_CXX_FLAGS=-D_GLIBCXX_DEBUG -D_GLIBCXX_ASSERTIONS" -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DBUILD_GAME_FLOW_TESTS=ON -DBUILD_PLAYER_ABILITY_TESTS=ON -DBUILD_REPOSITORY_TESTS=ON -DBUILD_RENDER_SMOKE_TESTS=ON
cmake --build build-test-debug
ctest --test-dir build-test-debug --output-on-failure

cmake -S . -B build-test-release -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DBUILD_GAME_FLOW_TESTS=ON -DBUILD_PLAYER_ABILITY_TESTS=ON -DBUILD_REPOSITORY_TESTS=ON -DBUILD_RENDER_SMOKE_TESTS=ON
cmake --build build-test-release
ctest --test-dir build-test-release --output-on-failure
```

Disable `BUILD_RENDER_SMOKE_TESTS` on hosts without a working desktop OpenGL
driver. The remaining seven cases run without creating a window. The render test
writes optional inspection captures into its build directory. No tests, fixture
enemies, screenshots, or debug shortcuts are enabled in the production game.
