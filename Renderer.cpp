#define GLAD_GL_IMPLEMENTATION
#include "Renderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

static GLuint kitchenTexture = 0;
static int kitchenWidth = 0;
static int kitchenHeight = 0;

bool loadKitchenBackground()
{
    if (kitchenTexture != 0) return true;

    int channels;
    const char* filename = GAME_ASSET_DIR "/kitchen-background.png";
    unsigned char* pixels = stbi_load(filename, &kitchenWidth, &kitchenHeight,
                                      &channels, STBI_rgb_alpha);
    if (!pixels) {
        std::cerr << "Could not load " << filename << ": "
                  << stbi_failure_reason() << '\n';
        return false;
    }

    glGenTextures(1, &kitchenTexture);
    glBindTexture(GL_TEXTURE_2D, kitchenTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kitchenWidth, kitchenHeight,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void releaseKitchenBackground()
{
    glDeleteTextures(1, &kitchenTexture);
    kitchenTexture = 0;
}

void drawKitchenBackground(GLint transformLocation, GLint colorLocation,
                           const glm::mat4& projection)
{
    if (kitchenTexture == 0) return;

    // The orthographic projection reveals extra space when the window resizes.
    float width = 2.0f / projection[0][0];
    float height = 2.0f / projection[1][1];
    float viewAspect = width / height;
    float imageAspect = float(kitchenWidth) / float(kitchenHeight);

    // Crop to fill the window while preserving the image's proportions.
    glm::vec2 crop(1.0f);
    if (viewAspect > imageAspect) crop.y = imageAspect / viewAspect;
    else crop.x = viewAspect / imageAspect;

    GLint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    GLint useTexture = glGetUniformLocation(program, "uUseTexture");
    GLfloat previousColor[3];
    glGetUniformfv(program, colorLocation, previousColor);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, kitchenTexture);
    glUniform1i(glGetUniformLocation(program, "uTexture"), 0);
    glUniform2f(glGetUniformLocation(program, "uTextureCrop"), crop.x, crop.y);
    glUniform1i(useTexture, GL_TRUE);
    glUniform3f(colorLocation, 1.0f, 1.0f, 1.0f);
    drawRectangle(transformLocation, 0.0f, 0.0f, width, height);

    // Existing characters, text and lanes still use solid colors.
    glUniform1i(useTexture, GL_FALSE);
    glUniform3fv(colorLocation, 1, previousColor);
    // Keep a complete texture bound for drivers that validate both shader branches.
}

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
