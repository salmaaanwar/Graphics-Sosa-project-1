cmake_minimum_required(VERSION 3.16)
project(OpenGLStarter)

add_subdirectory(external/glfw-3.5.1)
add_subdirectory(external/glm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

# Use the same image path for the game and optional rendering checks.
add_compile_definitions(GAME_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")

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
    PlayerCombat.cpp
    Deployment.cpp
    Enemy.cpp
    WaveManager.cpp
    Collectibles.cpp
)

target_include_directories(OpenGLStarter PRIVATE external/glad/include)
target_link_libraries(OpenGLStarter PRIVATE glfw OpenGL::GL glm::glm)

# Optional offline artwork export; the game itself only loads the resulting PNGs.
option(BUILD_KITCHEN_ASSET_EXPORTER "Export flat kitchen PNGs using the game's shape renderer" OFF)
if(BUILD_KITCHEN_ASSET_EXPORTER)
    get_target_property(EXPORT_GAME_SOURCES OpenGLStarter SOURCES)
    list(REMOVE_ITEM EXPORT_GAME_SOURCES main.cpp UI.cpp TextRenderer.cpp)
    add_executable(KitchenAssetExporter tools/ExportKitchenAssets.cpp tools/FlatUIArtwork.cpp tools/FlatTextArtwork.cpp ${EXPORT_GAME_SOURCES})
    target_include_directories(KitchenAssetExporter PRIVATE . external/glad/include)
    target_link_libraries(KitchenAssetExporter PRIVATE glfw OpenGL::GL glm::glm)
endif()
if(MINGW)
    target_link_options(OpenGLStarter PRIVATE -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic)
endif()

# Requires a desktop OpenGL driver, but never displays a test window.
option(BUILD_RENDER_SMOKE_TESTS "Build hidden-window graphics and callback checks" OFF)
if(BUILD_RENDER_SMOKE_TESTS)
    enable_testing()
    add_executable(RenderSmokeTests tests/RenderSmokeTests.cpp ExampleScene.cpp WaveManager.cpp Collectibles.cpp
    Game.cpp Enemy.cpp Cutscene.cpp Environment.cpp Player.cpp PlayerCombat.cpp
    Deployment.cpp UI.cpp Renderer.cpp TextRenderer.cpp)
    target_include_directories(RenderSmokeTests PRIVATE . external/glad/include)
    target_link_libraries(RenderSmokeTests PRIVATE glfw OpenGL::GL glm::glm)
    if(MINGW)
        target_link_options(RenderSmokeTests PRIVATE -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic)
    endif()
    add_test(NAME RenderSmoke COMMAND RenderSmokeTests "${CMAKE_CURRENT_BINARY_DIR}/render-captures")
    set_tests_properties(RenderSmoke PROPERTIES TIMEOUT 60)
endif()

option(BUILD_REPOSITORY_TESTS "Build movement, timing and input stress checks" OFF)
if(BUILD_REPOSITORY_TESTS)
    enable_testing()
    add_executable(RepositoryTests tests/RepositoryTests.cpp WaveManager.cpp Collectibles.cpp
        Game.cpp Enemy.cpp Cutscene.cpp Environment.cpp Player.cpp PlayerCombat.cpp
        Deployment.cpp UI.cpp Renderer.cpp TextRenderer.cpp)
    target_include_directories(RepositoryTests PRIVATE . external/glad/include)
    target_link_libraries(RepositoryTests PRIVATE glfw OpenGL::GL glm::glm)
    if(MINGW)
        target_link_options(RepositoryTests PRIVATE -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic)
    endif()
    foreach(scenario movement stress invalid-time long-frame frozen-states)
        add_test(NAME Repository-${scenario} COMMAND RepositoryTests ${scenario})
        set_tests_properties(Repository-${scenario} PROPERTIES TIMEOUT 30)
    endforeach()
endif()

option(BUILD_PLAYER_ABILITY_TESTS "Build Member 2's attack/deployment checks" OFF)
if(BUILD_PLAYER_ABILITY_TESTS)
    enable_testing()
    add_executable(PlayerAbilitiesTests
        tests/PlayerAbilitiesTests.cpp
        WaveManager.cpp
        Collectibles.cpp
        Game.cpp
        Enemy.cpp
        Cutscene.cpp
        Environment.cpp
        Player.cpp
        PlayerCombat.cpp
        Deployment.cpp
        UI.cpp
        Renderer.cpp
        TextRenderer.cpp
    )
    target_include_directories(PlayerAbilitiesTests PRIVATE . external/glad/include)
    target_link_libraries(PlayerAbilitiesTests PRIVATE glfw OpenGL::GL glm::glm)
    if(MINGW)
        target_link_options(PlayerAbilitiesTests PRIVATE -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic)
    endif()
    add_test(NAME PlayerAbilities COMMAND PlayerAbilitiesTests)
endif()

# Optional state/menu tests. They run without creating an OpenGL window.
option(BUILD_GAME_FLOW_TESTS "Build Member 4's game-flow checks" OFF)
if(BUILD_GAME_FLOW_TESTS)
    enable_testing()
    add_executable(GameFlowTests
        tests/GameFlowTests.cpp
        WaveManager.cpp
        Collectibles.cpp
        Game.cpp
        Enemy.cpp
        Cutscene.cpp
        Environment.cpp
        Player.cpp
        PlayerCombat.cpp
        Deployment.cpp
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
