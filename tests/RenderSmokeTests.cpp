// Compile the actual application's shaders and input callbacks. Only cursor
// sampling is replaced: tests inject world coordinates without moving the user's
// desktop cursor or bringing the hidden test window into focus.
#include "Renderer.h"
static glm::vec2 testMouse(-99.0f, -99.0f);
static bool testMouseToWorld(GLFWwindow*, const glm::mat4&, glm::vec2& world)
{
    world = testMouse;
    return true;
}
#define mouseToWorld testMouseToWorld
#define main gameApplicationMain
#include "../main.cpp"
#undef main
#undef mouseToWorld
#include <cstdlib>
#include <fstream>
#include <filesystem>

static void check(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAILED: " << message << '\n'; std::exit(1); }
}

int main(int argc, char** argv)
{
    check(glfwInit(), "GLFW initialization");
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Render smoke test", nullptr, nullptr);
    check(window != nullptr, "hidden OpenGL context creation");
    glfwMakeContextCurrent(window);
    check(gladLoadGL(glfwGetProcAddress) != 0, "GLAD initialization");
    std::cout << "GPU: " << glGetString(GL_RENDERER) << "; OpenGL: " << glGetString(GL_VERSION) << '\n';
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    check(vs != 0 && fs != 0, "production shaders compile");
    GLuint program = glCreateProgram();
    glAttachShader(program, vs); glAttachShader(program, fs); glLinkProgram(program);
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    check(linked != 0, "production shaders link");
    glUseProgram(program);

    std::vector<float> vertices = {-0.5f,-0.5f,0, 0.5f,-0.5f,0, 0.5f,0.5f,0,
                                  -0.5f,-0.5f,0, 0.5f,0.5f,0, -0.5f,0.5f,0};
    const int circleStart = 6, circleCount = 52;
    vertices.insert(vertices.end(), {0, 0, 0});
    for (int i = 0; i <= 50; i++) {
        float a = 6.2831853f * i / 50;
        vertices.insert(vertices.end(), {cosf(a), sinf(a), 0});
    }
    int triangleStart = (int)vertices.size() / 3;
    vertices.insert(vertices.end(), {-0.5f,-0.5f,0, 0.5f,-0.5f,0, 0,0.5f,0});
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao); glBindVertexArray(vao);
    glGenBuffers(1, &vbo); glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    GLint t = glGetUniformLocation(program, "u_Transform");
    GLint c = glGetUniformLocation(program, "uColor");
    GLint p = glGetUniformLocation(program, "u_Projection");
    int width = 0, height = 0, frames = 0;
    std::string output = argc > 1 ? argv[1] : "";
    if (!output.empty()) std::filesystem::create_directories(output);

    auto click = [&](float x, float y) {
        testMouse = glm::vec2(x, y);
        mouseButtonCallback(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
        mouseButtonCallback(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
    };
    auto render = [&](const std::string& name) {
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUniformMatrix4fv(p, 1, GL_FALSE, glm::value_ptr(projection));
        int state = getGameState();
        if (state == STATE_MAIN_MENU) drawUI(t, c, circleStart, circleCount, triangleStart, -99, -99);
        else if (state == STATE_CUTSCENE) drawCutscene(t, c, circleStart, circleCount, triangleStart);
        else {
            getEnvironment().render(t, c, circleStart, circleCount, triangleStart);
            drawPlayers(t, c, circleStart, circleCount, triangleStart);
            drawPlayerCombat(t, c, circleStart, circleCount);
            if (state == STATE_PLAYING || state == STATE_PAUSED)
                drawHUD(t, c, circleStart, circleCount, triangleStart, -99, -99);
            if (state == STATE_PAUSED) drawPauseMenu(t, c, -99, -99);
            if (state == STATE_WON || state == STATE_LOST)
                drawWinLoseMenu(t, c, circleStart, circleCount, triangleStart, state == STATE_WON, -99, -99);
        }
        glFinish();
        check(glGetError() == GL_NO_ERROR, "draw calls have no OpenGL errors");
        std::vector<unsigned char> pixels(width * height * 3);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
        check(glGetError() == GL_NO_ERROR, "framebuffer readback has no OpenGL errors");
        if (!output.empty() && !name.empty()) {
            std::ofstream image(output + "/" + name + ".ppm", std::ios::binary);
            image << "P6\n" << width << ' ' << height << "\n255\n";
            for (int row = height - 1; row >= 0; row--)
                image.write((const char*)pixels.data() + row * width * 3, width * 3);
        }
        frames++;
        return pixels;
    };

    for (const auto& size : {glm::ivec2(800,600), glm::ivec2(1280,720), glm::ivec2(600,800),
                             glm::ivec2(320,240), glm::ivec2(1920,1080)}) {
        glfwSetWindowSize(window, size.x, size.y);
        glfwPollEvents();
        glfwGetFramebufferSize(window, &width, &height);
        framebufferSizeCallback(window, width, height);
        GLint viewport[4]; glGetIntegerv(GL_VIEWPORT, viewport);
        check(viewport[2] == width && viewport[3] == height, "resize updates the framebuffer viewport");
        glm::mat4 before = projection;
        framebufferSizeCallback(window, 0, 0);
        check(projection == before, "zero-sized framebuffer preserves projection");
        std::string prefix = std::to_string(size.x) + "x" + std::to_string(size.y) + "-";
        returnToMainMenu();
        render(size.x == 800 ? "menu" : "");
        click(0, 0.02f);
        check(getGameState() == STATE_CUTSCENE, "actual Start callback opens the intro");
        for (int second = 0; second < 16; second += 2) {
            if (second > 0) updateGame(2);
            render(size.x == 800 ? "intro-" + std::to_string(second) : "");
        }
        updateGame(2);
        check(isGameplayRunning(), "intro transitions to gameplay");
        click(-0.74f, 0.84f);
        check(getSelectedCard() == PLAYER_MELEE, "actual card callback arms melee");
        click(1.2f, 0.43f);
        check(getCollectibles() == 30, "click outside the grid spends nothing");
        click(0, getEnvironment().getLaneCenterY(0));
        check(getPlayers().size() == 1 && getCollectibles() == 20, "actual lane callback buys one player");
        keyCallback(window, GLFW_KEY_W, 0, GLFW_PRESS, 0);
        check(getPlayers()[0].pose == POSE_JUMPING, "actual key callback jumps in the selected lane");
        updateGame(0.7f);
        setCollectibles(200);
        click(-0.44f, 0.84f);
        click(0, getEnvironment().getLaneCenterY(1));
        click(-0.14f, 0.84f);
        click(0, getEnvironment().getLaneCenterY(2));
        check(getPlayers().size() == 3 && getCollectibles() == 50, "spray and sheet purchases use real callbacks");
        updateGame(0.13f);
        render(prefix + "playing");
        click(0.78f, 0.84f);
        check(getGameState() == STATE_PAUSED, "HUD pause callback works");
        auto paused = render(size.x == 800 ? "paused" : "");
        updateGame(30);
        check(paused == render(""), "pause is pixel-identical after a 30-second update");
        click(0, 0.10f);
        check(isGameplayRunning(), "Continue callback resumes");
        finishGame(true);
        render(size.x == 800 ? "won" : "");
        click(0, -0.04f);
        check(isGameplayRunning() && getPlayers().empty() && getPlayerProjectiles().empty(),
              "result Restart callback clears real gameplay state");
        finishGame(false);
        render(size.x == 800 ? "lost" : "");
        click(0, -0.32f);
        check(getGameState() == STATE_MAIN_MENU, "result Main Menu callback works");
    }
    click(0, -0.28f);
    check(glfwWindowShouldClose(window), "Exit button closes the actual GLFW window");
    std::cout << frames << " frames passed across five sizes: shaders, rendering, callbacks, pause and reset.\n";
    glDeleteBuffers(1, &vbo); glDeleteVertexArrays(1, &vao);
    glDeleteShader(vs); glDeleteShader(fs); glDeleteProgram(program);
    glfwDestroyWindow(window); glfwTerminate();
}
