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