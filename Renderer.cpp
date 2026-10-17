#define GLAD_GL_IMPLEMENTATION
#include "Renderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//triangle drawing
void drawTriangle(
    GLint transformLocation,
    int firstVertex,
    float x,
    float y,
    float width,
    float height,
    float angleDegrees
){
    glm::mat4 transform = glm::mat4(1.0f);
    
    transform = glm::translate(
        transform, glm::vec3(x, y, 0.0f)
    );

    transform = glm::rotate(
        transform,
        glm::radians(angleDegrees),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );

    transform = glm::scale(
        transform, glm::vec3(width, height, 1.0f)
    );

    glUniformMatrix4fv(
        transformLocation, 1, GL_FALSE, glm::value_ptr(transform)
    );

    glDrawArrays(GL_TRIANGLES, firstVertex, 3);
}

//rectangle drawing
void drawRectangle(
    GLint transformLocation,
    float x,
    float y,
    float width,
    float height,
    float angleDegrees
){
    glm::mat4 transform = glm::mat4(1.0f);
    
    transform = glm::translate(
        transform, glm::vec3(x, y, 0.0f)
    );

    transform = glm::rotate(
        transform,
        glm::radians(angleDegrees),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );

    transform = glm::scale(
        transform, glm::vec3(width, height, 1.0f)
    );

    glUniformMatrix4fv(
        transformLocation, 1, GL_FALSE, glm::value_ptr(transform)
    );

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

//circle drawing
void drawCircle(
    GLint transformLocation,
    int firstVertex, 
    int vertexCount,
    float x, 
    float y,
    float radius
){
    glm::mat4 transform = glm::mat4(1.0f);

    transform = glm::translate(
        transform, glm::vec3(x, y, 0.0f)
    );

    transform = glm::scale(
        transform, glm::vec3(radius, radius, 1.0f)
    );

    glUniformMatrix4fv(
        transformLocation, 1, GL_FALSE, glm::value_ptr(transform)
    );

    glDrawArrays(GL_TRIANGLE_FAN, firstVertex, vertexCount);
}

//mouse-to-world conversion
bool mouseToWorld(
    GLFWwindow* window,
    const glm::mat4& projection,
    glm::vec2& worldPosition
){
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    if(width <= 0 || height <= 0){
        return false;
    }

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    float normalizedX = 2.0f * (float)mouseX / (float)width - 1.0f;
    float normalizedY = 1.0f - 2.0f * (float)mouseY / (float)height;

    glm::vec4 world = glm::inverse(projection) * glm::vec4(normalizedX, normalizedY, 0.0f, 1.0f);

    worldPosition = glm::vec2(world);

    return true;
}