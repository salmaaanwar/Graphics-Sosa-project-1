cmake_minimum_required(VERSION 3.16)
project(OpenGLStarter)

add_subdirectory(external/glfw-3.5.1)
add_subdirectory(external/glm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

# Use the same image path for the game and optional rendering checks.
add_compile_definitions(GAME_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")

# miniaudio 0.11.23, commit f40cf03f80cdb7e741d43e53b7e706e8c1394bcf.
# One implementation shared by the game, exporter, and all test targets.
find_package(Threads REQUIRED)
add_library(GameAudio STATIC Audio.cpp)
target_link_libraries(GameAudio PUBLIC Threads::Threads ${CMAKE_DL_LIBS})
if(WIN32)
    target_link_libraries(GameAudio PUBLIC winmm ole32 uuid)
elseif(APPLE)
    target_link_libraries(GameAudio PUBLIC "-framework CoreFoundation" "-framework CoreAudio" "-framework AudioToolbox")
elseif(UNIX)
    target_link_libraries(GameAudio PUBLIC m)
endif()
link_libraries(GameAudio)

add_executable(OpenGLStarter
    main.cpp
    Renderer.cpp
    ExampleScene.cpp
    TextRenderer.cpp
    UI.cpp
    AudioControls.cpp
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
    list(REMOVE_ITEM EXPORT_GAME_SOURCES main.cpp UI.cpp AudioControls.cpp TextRenderer.cpp)
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
    Deployment.cpp UI.cpp AudioControls.cpp Renderer.cpp TextRenderer.cpp)
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
        Deployment.cpp UI.cpp AudioControls.cpp Renderer.cpp TextRenderer.cpp)
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
        AudioControls.cpp
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
        AudioControls.cpp
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

# Package audio next to the executable, independent of the launch directory.
add_custom_command(TARGET OpenGLStarter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/audio"
        "$<TARGET_FILE_DIR:OpenGLStarter>/assets/audio")
# Also refresh changed assets when a default build doesn't need to re-link.
# TARGET_FILE_DIR creates a dependency on the executable, so no reverse
# add_dependencies() is needed here (which would create a cycle).
add_custom_target(RefreshGameAudio ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/audio"
        "$<TARGET_FILE_DIR:OpenGLStarter>/assets/audio")

option(BUILD_AUDIO_TESTS "Build audio event, control and MP3 decoding checks" OFF)
if(BUILD_AUDIO_TESTS)
    enable_testing()
    add_executable(AudioTests tests/AudioTests.cpp WaveManager.cpp Collectibles.cpp
        Game.cpp Enemy.cpp Cutscene.cpp Environment.cpp Player.cpp PlayerCombat.cpp
        Deployment.cpp UI.cpp AudioControls.cpp Renderer.cpp TextRenderer.cpp)
    target_include_directories(AudioTests PRIVATE . external/glad/include)
    target_link_libraries(AudioTests PRIVATE glfw OpenGL::GL glm::glm)
    add_test(NAME AudioEvents COMMAND AudioTests)
    add_test(NAME AudioAssets COMMAND AudioTests --validate "${CMAKE_CURRENT_SOURCE_DIR}/assets/audio")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/audio-working-directory")
    add_dependencies(AudioTests OpenGLStarter)
    add_test(NAME AudioPackagedPlayback COMMAND AudioTests "$<TARGET_FILE_DIR:OpenGLStarter>")
    set_tests_properties(AudioPackagedPlayback PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/audio-working-directory" TIMEOUT 60)
    set_tests_properties(AudioEvents AudioAssets PROPERTIES TIMEOUT 60)
    add_executable(AudioControlsRenderTests tests/AudioControlsRenderTests.cpp
        WaveManager.cpp Collectibles.cpp Game.cpp Enemy.cpp Cutscene.cpp Environment.cpp
        Player.cpp PlayerCombat.cpp Deployment.cpp UI.cpp AudioControls.cpp Renderer.cpp
        TextRenderer.cpp ExampleScene.cpp)
    target_include_directories(AudioControlsRenderTests PRIVATE . external/glad/include)
    target_link_libraries(AudioControlsRenderTests PRIVATE glfw OpenGL::GL glm::glm)
    add_test(NAME AudioControlsRender COMMAND AudioControlsRenderTests "${CMAKE_CURRENT_BINARY_DIR}/audio-captures")
    set_tests_properties(AudioControlsRender PROPERTIES TIMEOUT 60)
endif()
