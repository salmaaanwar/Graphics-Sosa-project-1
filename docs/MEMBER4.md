# Member 4: Cutscene and game flow

The normal launch sequence is Main Menu -> Start -> 16-second cutscene -> Playing.
The story follows the existing kitchen theme: insects want the family's dinner,
and a fly-swatter guard protects the food while a larger swarm approaches.

## What was added, and why

| File | Responsibility | Reason |
| --- | --- | --- |
| `Cutscene.h` | Intro duration and reset/update/draw declarations | Lets the main loop use the intro without knowing its drawing details |
| `Cutscene.cpp` | Timer, animated characters, kitchen scenery and subtitles | Keeps your cinematic work in its own file |
| `Game.h` | Shared state constants and transition functions | Gives the UI and future gameplay modules the same state definitions |
| `Game.cpp` | State storage, session timer, pause/restart/result behavior | Keeps all transitions in one place instead of separate UI and game copies |
| `tests/GameFlowTests.cpp` | Checks timing and actual UI input handlers | Catches cases such as Start skipping the intro or Continue resetting the session |

The existing shared files have small integration changes:

- `main.cpp` calculates delta time, calls `updateGame`, and draws either the intro or the existing UI. V-sync follows the Lab 4 example.
- `UI.h` includes `Game.h` instead of defining a second set of state constants. Other files can still include `UI.h` and use the same names.
- `UI.cpp` calls `startNewGame`, `pauseGame`, `resumeGame`, `restartGame`, `returnToMainMenu` and `finishGame`. Its static intro placeholder and Continue button were removed. Gameplay test input is ignored while paused or finished.
- `CMakeLists.txt` compiles the two new source files and optionally builds the flow tests.

## How the code follows the labs

The module uses ordinary functions, file-level variables, constants, loops and
`if`/`else` statements. It uses the existing OpenGL shader, VAO and VBO. Shapes
still go through `drawRectangle`, `drawCircle` and `drawTriangle`.

The `static` variables at the top of the new `.cpp` files are like the lab's
global position/camera variables, but their names are private to their file.
Other files access the timer/state through functions.

The intro reuses `drawSwatterIcon` from the UI. Its eyes, arms and feet are extra
primitives drawn around that icon. The fly is another composite placeholder:
separate wings, body parts, eye, legs and antenna. Its own drawing helper lets
the wings animate without changing Member 3's menu insect.

## 1. Timing: update once, draw once

Lab 4 calculates the time between frames. The same calculation is now in `main.cpp`:

```cpp
float currentFrame = static_cast<float>(glfwGetTime());
float deltaTime = currentFrame - lastFrame;
lastFrame = currentFrame;
updateGame(deltaTime);
```

`glfwGetTime()` returns seconds. At roughly 60 FPS, `deltaTime` is about 0.0167
seconds. Adding it to `cutsceneTime` measures actual elapsed time instead of
counting frames. A computer drawing 144 FPS therefore does not finish the story
faster than one drawing 30 FPS.

Three functions have separate jobs:

```cpp
resetCutscene();           // Set elapsed time to zero.
updateCutscene(deltaTime); // Advance the timer by elapsed seconds.
drawCutscene(...);         // Draw the scene at the current timer value.
```

The draw function does not advance time. Drawing the same scene twice cannot
make the animation run twice as fast. `updateGame` advances the intro only in
`STATE_CUTSCENE`, then changes to Playing when its timer reaches 16 seconds.

## 2. Moving between two positions

`sceneProgress(start, end)` returns a number between 0 and 1:

```cpp
progress = (cutsceneTime - startTime) / (endTime - startTime);
```

Clamping keeps the result at 0 before the interval and 1 after it. This stops an
object from continuing past its destination.

`glm::mix(startPosition, endPosition, progress)` performs linear interpolation:

```cpp
float swatterX = glm::mix(-0.38f, -0.12f, sceneProgress(6.0f, 9.0f));
```

At 6 seconds the guard is at -0.38. At 7.5 seconds progress is 0.5, so it is
halfway at -0.25. At 9 seconds it reaches -0.12 and stops advancing.

Sine produces repeated movement:

```cpp
float flap = std::sin(cutsceneTime * 30.0f);
```

Its output moves between -1 and 1. Multiplying that by an angle makes the wings
flap. A smaller multiplier gives the guard a gentle bob. During the block,
`sin(progress * pi)` gives one forward-and-back motion rather than an endless
oscillation. These are the same time/trigonometry ideas used in the Lab 4 demo.

## 3. The story timeline

| Seconds | Visible action | Story purpose |
| --- | --- | --- |
| 0-3 | Food steams; the guard idles beside the plate | Establish the kitchen and the family's dinner |
| 3-6 | A fly enters from the right, with flapping wings | Introduce the threat and its intention |
| 6-9 | The guard advances in front of the food | Explain what is being protected and why |
| 9-12 | The fly lunges; the guard blocks; a contact flash appears; the fly retreats | Show an interaction through animation as well as dialogue |
| 12-16 | More insects arrive; the guard prepares to defend the lanes | Connect the story to lane-defense gameplay |

Subtitles use the existing uppercase bitmap text renderer. Backgrounds are
drawn first, characters next and text last, so text remains readable. The
important content fits inside the renderer's central -1 to +1 square.

There is no normal Continue/Skip button: Start plays the full intro and then
automatically enters gameplay. Escape is reserved for pausing gameplay.

## 4. Game flow and reset behavior

| Action | Result | Data behavior |
| --- | --- | --- |
| Start | Cutscene, then Playing | Reset placeholders and both timers |
| Escape / HUD Pause | Paused | Preserve the current session |
| Continue / Escape while paused | Playing | Preserve values and resume the gameplay timer |
| Restart | Playing directly | Reset placeholders, selected card and timers |
| Main Menu | Main Menu | Reset session values and intro timer |
| `finishGame(true)` | Won | Stop gameplay updates |
| `finishGame(false)` | Lost | Stop gameplay updates |

Restart deliberately avoids replaying the introduction. Start from the main
menu always replays it. This is an implementation choice, not an extra rubric
requirement.

`updateGame` advances `gameplayTime` only while Playing. It keeps drawing the
appropriate scene/menu during Paused, Won and Lost, but does not advance gameplay.
This is why pause does not need a second render loop or a blocking `while` loop.

When player/enemy modules are ready, put their updates in the Playing branch of
`updateGame`, or gate their update calls with `isGameplayRunning()`. Their inputs
must use the same check. That ensures movement, attacks and spawning all stop
when paused or lost.

The gameplay modules should call `finishGame(true)` after every enemy in all
three waves is defeated, and `finishGame(false)` when an enemy reaches the home
base. The flow controller handles the resulting screen; it does not invent wave
or collision logic before those modules exist.

The current reset calls `resetGamePlaceholders()` because the UI owns the test
coins and health. Add the real player/enemy/environment reset functions here
when the team integrates those modules. Continue must not call those resets.

## 5. Try it and inspect it

Build normally, launch `OpenGLStarter`, and click Start. Watch the full 16 seconds.
Then test Escape, Continue, Restart, and Main Menu. The existing development
shortcuts are still available for independent module testing:

- `C`: start a fresh intro.
- `1`: return to Main Menu; `2`: reset directly into Playing.
- `3`: pause Playing; `4` / `5`: finish a Playing session as Won / Lost.
- Up/Down or +/-: change test coins while Playing; `H`: test health damage.
- `R`: restart Playing.

These shortcuts are test controls, including `2` which bypasses the intro.
Remove the development shortcuts when real gameplay is ready for submission.

To enable the automated checks with CMake on PATH:

```powershell
cmake -S . -B build -DBUILD_GAME_FLOW_TESTS=ON
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

The tests exercise the real menu input functions without opening an OpenGL
window. They cover intro boundaries at 30/60/144 FPS, old intro-button clicks,
pause/resume timing, HUD pause, resets, both result menus and invalid states.

Implementation verification also rendered 80 intro frames and all five UI
screens in a hidden OpenGL window, plus portrait/widescreen intro captures.
There were no shader compilation or OpenGL errors in that check. The local
`build/previews/cutscene-preview.gif` and `cutscene-storyboard.png` show the result.
These generated previews are ignored build artifacts, not required game assets.
