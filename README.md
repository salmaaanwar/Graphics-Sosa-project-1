Renderer Guide

files:
    - main.cpp - window, shaders, geometry, projection, and main loop
    - Renderer.h, Renderer.cpp - shapes, transformations, and mouse conversion.
    - TextRenderer.h, TextRenderer.cpp - text and number drawing
    - ExampleScene.h, ExampleScene.cpp - working examples to follow
    - CMakeLists.txt - source files and library configuration

build and run:
run these commands from the project folder
for macOS 
    cmake -S . -B build
    cmake --build build
    ./build/OpenGLStarter

for windows ->
    cmake -S . -B build
    cmake --build build
    .\build\OpenGLStarter.exe

    If using Visual Studio’s build system, the executable is usually in the Debug folder. Use this launch command instead:

    .\build\Debug\OpenGLStarter.exe


using the renderer:

include the headers your module needs in the .cpp file:
    #include "Renderer.h"
    #include "TextRenderer.h"

set a color before drawing, for example:
    glUniform3f(colorLocation, 1.0f, 0.0f, 0.0f);
    drawRectangle(transformLocation, 0.0f, 0.0f, 0.4f, 0.2f); 

available functions:
    - drawRectangle - center, full width/height, optional rotation.
    - drawCircle - center, radius, and geometry range.
    - drawTriangle - local-origin position, width/height, geometry range, optional rotation.
    - drawDigit, drawNumber, drawText - top-left position and cell size. text supports uppercase letters, digits, and spaces; numbers must be nonnegative.
    - mouseToWorld - converts cursor position into world coordinates; returns whether conversion succeeded.

follow ExampleScene.cpp for complete drawing calls. pass the required uniform locations and geometry indices from main.cpp into your module

where to implement your part (these are just suggestions):
    1. renderer -> Renderer.h, Renderer.cpp, TextRenderer.h, TextRenderer.cpp
    2. Environment and lanes -> Environment.h, Environment.cpp
    3. Menus and HUD -> UI.h, UI.cpp
    4. Cutscene and game flow -> Cutscene.h, Cutscene.cpp, Game.h, Game.cpp

create your module files, declare functions in the header (.h) and implement them in the .cpp.

add each new .cpp to add_executable(...) in CMakeLists.txt. 

in main.cpp, call the active scene's drawing functions after glBindVertexArray(VAO) and before glfwSwapBuffers(window).

find const bool showExamples = true; immediately before the main render loop in main.cpp. set it to true to display the examples, or false to hide the demonstrations when integrating the actual scenes.

