cmake_minimum_required(VERSION 3.10)
project(OpenGLStarter)

add_subdirectory(external/glfw-3.5.1)
add_subdirectory(external/glm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

add_executable(OpenGLStarter
    main.cpp
    Renderer.cpp
    ExampleScene.cpp
    TextRenderer.cpp
    UI.cpp
    Game.cpp
    Cutscene.cpp
    Environment.cpp
    Player.cpp
)

target_include_directories(OpenGLStarter PRIVATE external/glad/include)
target_link_libraries(OpenGLStarter PRIVATE glfw OpenGL::GL glm::glm)
if(MINGW)
    target_link_options(OpenGLStarter PRIVATE -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic)
endif()

# Optional state/menu tests. They run without creating an OpenGL window.
option(BUILD_GAME_FLOW_TESTS "Build Member 4's game-flow checks" OFF)
if(BUILD_GAME_FLOW_TESTS)
    enable_testing()
    add_executable(GameFlowTests
        tests/GameFlowTests.cpp
        Game.cpp
        Cutscene.cpp
        Environment.cpp
        Player.cpp
        UI.cpp
        Renderer.cpp
        TextRenderer.cpp
    )
    target_include_directories(GameFlowTests PRIVATE . external/glad/include)
    target_link_libraries(GameFlowTests PRIVATE glfw OpenGL::GL glm::glm)
    if(MINGW)
        target_link_options(GameFlowTests PRIVATE -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic)
    endif()
    add_test(NAME GameFlow COMMAND GameFlowTests)
endif()
