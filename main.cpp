#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include "Renderer.h"
#include "ExampleScene.h"
#include "UI.h"
#include "Game.h"
#include "Cutscene.h"
#include "Environment.h"
#include "Player.h"

glm::mat4 projection = glm::mat4(1.0f);

// vertex shader
const char *vertexShaderSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 u_Transform;
uniform mat4 u_Projection;
void main() {
    gl_Position = u_Projection * u_Transform * vec4(aPos, 1.0);
}
)";

// fragment shader
const char *fragmentShaderSrc = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 uColor;
void main() {
    FragColor = vec4(uColor, 1.0);
}
)";

// shader compilation
GLuint compileShader(GLenum type, const char *src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error:\n"
                  << log << "\n";

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

// window resizing and projection
void framebufferSizeCallback(
    GLFWwindow *window,
    int width,
    int height)
{
    if (width <= 0 || height <= 0)
    {
        return;
    }

    glViewport(0, 0, width, height);

    float aspect = (float)width / (float)height;

    if (aspect >= 1.0f)
    {
        projection = glm::ortho(
            -aspect, aspect,
            -1.0f, 1.0f);
    }
    else
    {
        projection = glm::ortho(
            -1.0f, 1.0f,
            -1.0f / aspect,
            1.0f / aspect);
    }
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    handleUIKeyPress(window, key, action);
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        glm::vec2 mouseWorld;
        if (mouseToWorld(window, projection, mouseWorld))
        {
            getEnvironment().handleMouseButton(button, action, mouseWorld.x, mouseWorld.y);
            // The playing field absorbs clicks below the HUD so UI's broad placeholder
            // placement zone cannot spend coins on empty space or duplicate lane clicks.
            if (action == GLFW_PRESS &&
                !(getGameState() == STATE_PLAYING && mouseWorld.y < 0.65f))
            {
                handleUIMouseClick(window, mouseWorld.x, mouseWorld.y);
            }
        }
    }
}

int main()
{

    // window and opengl initialization
    if (!glfwInit())
    {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "game", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Match Lab 4: avoid drawing more frames than the display needs.

    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0){
        std::cerr << "Failed to initialize OpenGL context\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // initial viewport and resize callback
    int framebufferWidth;
    int framebufferHeight;

    glfwGetFramebufferSize(
        window,
        &framebufferWidth,
        &framebufferHeight);

    framebufferSizeCallback(window, framebufferWidth, framebufferHeight);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    // --- Shader program ---
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

    if (vertexShader == 0 || fragmentShader == 0)
    {
        if (vertexShader != 0)
        {
            glDeleteShader(vertexShader);
        }

        if (fragmentShader != 0)
        {
            glDeleteShader(fragmentShader);
        }

        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, log);
        std::cerr << "Shader link error:\n"
                  << log << "\n";

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(shaderProgram);

        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // rectangle vertices
    std::vector<float> vertices{
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f};

    // circle vertices
    const int circleVertexStart = 6;
    const int segments = 50;

    vertices.insert(vertices.end(), {0.0f, 0.0f, 0.0f});

    for (int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * 3.14159265 * (float)i / (float)segments;

        vertices.insert(vertices.end(), {cosf(angle), sinf(angle), 0.0f});
    }

    const int circleVertexCount = segments + 2;

    // triangle vertices
    const int triangleVertexStart = static_cast<int>(vertices.size() / 3);

    vertices.insert(vertices.end(), {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f});

    // VAO and VBO setup
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // uniform locations
    GLint transformLocation = glGetUniformLocation(shaderProgram, "u_Transform");
    GLint colorLocation = glGetUniformLocation(shaderProgram, "uColor");
    GLint projectionLocation = glGetUniformLocation(shaderProgram, "u_Projection");

    const bool showExamples = false;
    double lastFrame = glfwGetTime();
    Environment& environment = getEnvironment();

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        double currentFrame = glfwGetTime();
        float deltaTime = static_cast<float>(currentFrame - lastFrame);
        lastFrame = currentFrame;

        if (!showExamples) updateGame(deltaTime);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);

        glm::vec2 mouseWorld(-99.0f, -99.0f);
        mouseToWorld(window, projection, mouseWorld);
        environment.handleMouseMove(mouseWorld.x, mouseWorld.y);

        if (showExamples)
        {
            drawExampleScene(
                transformLocation,
                colorLocation,
                circleVertexStart,
                circleVertexCount,
                triangleVertexStart);

            if (mouseToWorld(window, projection, mouseWorld))
            {
                glUniform3f(colorLocation, 1.0f, 1.0f, 1.0f);

                drawCircle(
                    transformLocation,
                    circleVertexStart,
                    circleVertexCount,
                    mouseWorld.x,
                    mouseWorld.y,
                    0.025f);
            }
        }
        else
        {
            const int state = getGameState();
            if (state == STATE_CUTSCENE)
            {
                drawCutscene(transformLocation, colorLocation, circleVertexStart,
                    circleVertexCount, triangleVertexStart);
            }
            else if (state == STATE_PLAYING)
            {
                environment.render(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart);
                drawPlayers(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart);
                drawHUD(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                        mouseWorld.x, mouseWorld.y);
            }
            else if (state == STATE_PAUSED)
            {
                environment.render(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart);
                drawPlayers(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart);
                drawHUD(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart, -99.0f, -99.0f);
                drawPauseMenu(transformLocation, colorLocation, mouseWorld.x, mouseWorld.y);
            }
            else if (state == STATE_WON || state == STATE_LOST)
            {
                environment.render(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart);
                drawPlayers(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart);
                drawWinLoseMenu(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                                state == STATE_WON, mouseWorld.x, mouseWorld.y);
            }
            else
            {
                drawUI(transformLocation, colorLocation, circleVertexStart, circleVertexCount, triangleVertexStart,
                       mouseWorld.x, mouseWorld.y);
            }
        }

        glfwSwapBuffers(window);
    }

    // cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
