cmake_minimum_required(VERSION 3.16)
project(OpenGLStarter)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(external/glfw-3.5.1)
add_subdirectory(external/glm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

add_compile_definitions(GAME_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")

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

if(MINGW)
    target_link_options(OpenGLStarter PRIVATE -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic)
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
