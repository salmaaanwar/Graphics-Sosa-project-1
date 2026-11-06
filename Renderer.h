#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// Kitchen image shared by the menu, intro and battlefield.
bool loadKitchenBackground();
void releaseKitchenBackground();
void drawKitchenBackground(GLint transformLocation, GLint colorLocation,
                           const glm::mat4& projection);

//triangle drawing
void drawTriangle(
    GLint transformLocation,
    int firstVertex,
    float x,
    float y,
    float width,
    float height,
    float angleDegrees = 0.0f
);

//rectangle drawing
void drawRectangle(
    GLint transformLocation,
    float x,
    float y,
    float width,
    float height,
    float angleDegrees = 0.0f
);

//circle drawing
void drawCircle(
    GLint transformLocation,
    int firstVertex, 
    int vertexCount,
    float x, 
    float y,
    float radius
);

//mouse-to-world conversion
bool mouseToWorld(
    GLFWwindow* window,
    const glm::mat4& projection,
    glm::vec2& worldPosition
);
