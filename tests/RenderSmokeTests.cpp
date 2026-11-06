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
    check(loadKitchenBackground(), "kitchen image loads from the configured asset directory");
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
        drawKitchenBackground(t, c, projection);
        drawKitchenDecorations(t, c, projection);
        int state = getGameState();
        if (state == STATE_MAIN_MENU) drawUI(t, c, circleStart, circleCount, triangleStart, testMouse.x, testMouse.y);
        else if (state == STATE_CUTSCENE) {
            drawCutscene(t, c, circleStart, circleCount, triangleStart);
            drawCutsceneSkipButton(t, c, testMouse.x, testMouse.y);
        }
        else {
            getEnvironment().render(t, c, circleStart, circleCount, triangleStart);
            drawEnemies(t, c, circleStart, circleCount, triangleStart);
            drawPlayers(t, c, circleStart, circleCount, triangleStart);
            drawPlayerCombat(t, c, circleStart, circleCount);
            drawCollectibles(t, c, circleStart, circleCount);
            if (state == STATE_PLAYING || state == STATE_PAUSED)
                drawHUD(t, c, circleStart, circleCount, triangleStart, state==STATE_PLAYING ? testMouse.x : -99, state==STATE_PLAYING ? testMouse.y : -99);
            if (state == STATE_PLAYING) drawWaveStatus(t, c);
            if (state == STATE_PAUSED) drawPauseMenu(t, c, testMouse.x, testMouse.y);
            if (state == STATE_WON || state == STATE_LOST)
                drawWinLoseMenu(t, c, circleStart, circleCount, triangleStart, state == STATE_WON, testMouse.x, testMouse.y);
        }
        glFinish();
        check(glGetError() == GL_NO_ERROR, "draw calls have no OpenGL errors");
        std::vector<unsigned char> pixels(width * height * 3);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
        check(glGetError() == GL_NO_ERROR, "framebuffer readback has no OpenGL errors");
        // Every corner must be covered even in portrait and ultrawide windows.
        for (int corner : {0, width - 1, (height - 1) * width, width * height - 1}) {
            const unsigned char* pixel = pixels.data() + corner * 3;
            bool clearColor = std::abs(int(pixel[0]) - 26) <= 1 &&
                              std::abs(int(pixel[1]) - 26) <= 1 &&
                              std::abs(int(pixel[2]) - 38) <= 1;
            check(!clearColor, "kitchen covers the window corners without empty borders");
        }
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
                             glm::ivec2(320,240), glm::ivec2(1920,1080),
                             glm::ivec2(2560,1080), glm::ivec2(800,800)}) {
        glfwSetWindowSize(window, size.x, size.y);
        glfwPollEvents();
        glfwGetFramebufferSize(window, &width, &height);
        framebufferSizeCallback(window, width, height);
        GLint viewport[4]; glGetIntegerv(GL_VIEWPORT, viewport);
        check(viewport[2] == width && viewport[3] == height, "resize updates the framebuffer viewport");
        Environment& environment = getEnvironment();
        float halfWidth = 1.0f / projection[0][0];
        check(std::abs(environment.getLaneStartX() - (-halfWidth + 0.32f)) < 0.0001f &&
              std::abs(environment.getLaneEndX() - (halfWidth - 0.08f)) < 0.0001f,
              "playable lanes extend to the actual window edges");
        check(environment.getLaneAtPoint(environment.getLaneEndX() - 0.01f,
                                        environment.getLaneCenterY(0)) == 0,
              "extended lane area remains clickable");
        // Verify that all five separate images contribute visible pixels over
        // the plain background and stay below the playable lanes.
        glUniformMatrix4fv(p, 1, GL_FALSE, glm::value_ptr(projection));
        glClear(GL_COLOR_BUFFER_BIT);
        drawKitchenBackground(t, c, projection);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        std::vector<unsigned char> background(width * height * 3);
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, background.data());
        drawKitchenDecorations(t, c, projection);
        std::vector<unsigned char> decorated(width * height * 3);
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, decorated.data());
        int changedPixels[5] = {};
        int footerRows = int(std::ceil(0.25f / (2.0f * environment.getViewHalfHeight()) * height));
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int pixel = (y * width + x) * 3;
                bool changed = background[pixel] != decorated[pixel] ||
                               background[pixel + 1] != decorated[pixel + 1] ||
                               background[pixel + 2] != decorated[pixel + 2];
                if (!changed) continue;
                check(y < footerRows, "decorations stay outside the playable lanes");
                ++changedPixels[std::min(4, x * 5 / width)];
            }
        }
        for (int count : changedPixels)
            check(count > 25, "each of the five separate kitchen images is visible");
        glm::mat4 before = projection;
        framebufferSizeCallback(window, 0, 0);
        check(projection == before, "zero-sized framebuffer preserves projection");
        std::string prefix = std::to_string(size.x) + "x" + std::to_string(size.y) + "-";
        returnToMainMenu();
        testMouse=glm::vec2(-99);
        auto menuNormal=render(prefix + "menu");
        testMouse=glm::vec2(0,.02f);
        check(menuNormal!=render(prefix+"menu-hover"),"main menu button image changes on hover");
        click(0, 0.02f);
        check(getGameState() == STATE_CUTSCENE, "actual Start callback opens the intro");
        for (int second = 0; second < 16; second += 2) {
            if (second > 0) updateGame(2);
            render(prefix + "intro-" + std::to_string(second));
        }
        updateGame(2);
        check(isGameplayRunning(), "intro transitions to gameplay");
        click(getEnvironment().getLaneEndX() + 0.10f, 0.43f);
        check(getCollectibles() == 30, "click outside the grid spends nothing");
        click(0, getEnvironment().getLaneCenterY(0));
        check(getPlayers().empty(), "selecting a lane does not spend coins");
        click(-getEnvironment().getViewHalfWidth() + 0.26f, 0.84f);
        check(getPlayers().size() == 1 && getCollectibles() == 20, "card click deploys into the selected lane");
        keyCallback(window, GLFW_KEY_W, 0, GLFW_PRESS, 0);
        check(getPlayers()[0].pose == POSE_JUMPING, "actual key callback jumps in the selected lane");
        updateGame(0.7f);
        for (int i = 0; i < 3; ++i) {
            dropCollectible(getPlayers()[0].x, getPlayers()[0].y, 10);
        }
        updateCollectibles();
        setCollectibles(200);
        click(0, getEnvironment().getLaneCenterY(0));
        click(0, getEnvironment().getLaneCenterY(1));
        click(-getEnvironment().getViewHalfWidth() + 0.56f, 0.84f);
        click(0, getEnvironment().getLaneCenterY(1));
        click(0, getEnvironment().getLaneCenterY(2));
        click(-getEnvironment().getViewHalfWidth() + 0.86f, 0.84f);
        check(getPlayers().size() == 3 && getCollectibles() == 200 - RANGED_COST - DEFENSE_COST, "spray and sheet purchases use real callbacks");
        updateGame(0.13f);
        render(prefix + "playing");
        updateGame(2.5f);
        render(prefix + "battle");
        click(getEnvironment().getViewHalfWidth() - 0.22f, 0.84f);
        check(getGameState() == STATE_PAUSED, "HUD pause callback works");
        auto paused = render(prefix+"paused");
        updateGame(30);
        check(paused == render(""), "pause is pixel-identical after a 30-second update");
        testMouse=glm::vec2(0,.10f);
        check(paused!=render(prefix+"paused-hover"),"pause button image changes on hover");
        click(0, 0.10f);
        check(isGameplayRunning(), "Continue callback resumes");
        // Resizing must move units, projectiles and drops together.
        spawnEnemy(ENEMY_EASY, 3);
        float oldStart = environment.getLaneStartX();
        float oldLength = environment.getLaneEndX() - oldStart;
        float playerProgress = (getPlayers()[0].x - oldStart) / oldLength;
        float enemyProgress = (getEnemies()[0].x - oldStart) / oldLength;
        check(!getPlayerProjectiles().empty(), "resize fixture includes an active projectile");
        float shotProgress = (getPlayerProjectiles()[0].x - oldStart) / oldLength;
        dropCollectible(getPlayers()[0].x, getPlayers()[0].y, 10);
        int coinsBeforeResize = getCollectibles();
        framebufferSizeCallback(window, width * 2, height);
        float newStart = environment.getLaneStartX();
        float newLength = environment.getLaneEndX() - newStart;
        check(std::abs((getPlayers()[0].x - newStart) / newLength - playerProgress) < 0.0001f &&
              std::abs((getEnemies()[0].x - newStart) / newLength - enemyProgress) < 0.0001f &&
              std::abs((getPlayerProjectiles()[0].x - newStart) / newLength - shotProgress) < 0.0001f,
              "resizing preserves relative positions of players, enemies and projectiles");
        updateCollectibles();
        check(getCollectibles() == coinsBeforeResize + 10, "drops stay aligned with players after resizing");
        framebufferSizeCallback(window, width, height);
        finishGame(true);
        testMouse=glm::vec2(-99);
        auto wonNormal=render(prefix+"won");
        testMouse=glm::vec2(0,-.04f);
        check(wonNormal!=render(prefix+"won-hover"),"win menu button image changes on hover");
        click(0, -0.04f);
        check(isGameplayRunning() && getPlayers().empty() && getPlayerProjectiles().empty(),
              "result Restart callback clears real gameplay state");
        finishGame(false);
        testMouse=glm::vec2(-99);
        auto lostNormal=render(prefix+"lost");
        testMouse=glm::vec2(0,-.32f);
        check(lostNormal!=render(prefix+"lost-hover"),"lose menu button image changes on hover");
        click(0, -0.32f);
        check(getGameState() == STATE_MAIN_MENU, "result Main Menu callback works");
    }
    restartGame();
    updateWaves(2.0f);
    resetEnemies();
    for(int type=0;type<3;++type) {
        int index=spawnPlayer(type,type);
        getPlayers()[index].x=-0.35f;
        getPlayers()[index].attackAnimation=0.12f;
        spawnEnemy(type,type);
        getEnemies().back().x=0.45f;
        getEnemies().back().attackAnimation=0.12f;
    }
    render("all-character-images");
    for(Player& player : getPlayers()) { player.hp-=2; if(player.type!=PLAYER_DEFENSE) player.pose=POSE_DUCKING; }
    for(Enemy& enemy : getEnemies()) enemy.hp-=2;
    render("duck-character-images");
    returnToMainMenu();
    click(0, 0.02f);
    click(0.80f, 0.86f);
    check(isGameplayRunning(), "actual Skip callback starts gameplay");
    render("wave-announcement");
    returnToMainMenu();
    click(0, -0.28f);
    check(glfwWindowShouldClose(window), "Exit button closes the actual GLFW window");
    std::cout << frames << " frames passed across seven sizes: kitchen coverage, shaders, rendering, callbacks, pause and reset.\n";
    releaseKitchenBackground();
    glDeleteBuffers(1, &vbo); glDeleteVertexArrays(1, &vao);
    glDeleteShader(vs); glDeleteShader(fs); glDeleteProgram(program);
    glfwDestroyWindow(window); glfwTerminate();
}
