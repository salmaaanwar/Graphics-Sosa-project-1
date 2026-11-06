#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

// Kitchen image shared by the menu, intro and battlefield.
bool loadKitchenBackground();
void releaseKitchenBackground();
void drawKitchenBackground(GLint transformLocation, GLint colorLocation,
                           const glm::mat4& projection);
void drawKitchenDecorations(GLint transformLocation, GLint colorLocation,
                            const glm::mat4& projection);
void drawKitchenCounter(GLint transformLocation, float x, float y, float width, float height);

enum FoodPicture
{
    FOOD_BREAD,
    FOOD_APPLE,
    FOOD_CHEESE,
    FOOD_PICTURE_COUNT
};
void drawFoodPlate(GLint transformLocation, GLint colorLocation, FoodPicture food, float x, float y,
                   float size = 0.235f);

// rectangle drawing
void drawRectangle(GLint transformLocation, float x, float y, float width, float height,
                   float angleDegrees = 0.0f);

// mouse-to-world conversion
bool mouseToWorld(GLFWwindow* window, const glm::mat4& projection, glm::vec2& worldPosition);

void drawIntroImage(GLint transformLocation, GLint colorLocation, const std::string& name, float x,
                    float y, float width, float height);

void drawCharacterImage(GLint transformLocation, GLint colorLocation, const std::string& name,
                        float x, float y, float damage);

void drawPictureTinted(GLint transformLocation, const std::string& name, float x, float y,
                       float width, float height);
void drawFontGlyph(GLint transformLocation, int character, float x, float y, float cellSize);

void drawLabelImage(GLint transformLocation, const std::string& label, float centerX, float centerY,
                    float cellSize);
