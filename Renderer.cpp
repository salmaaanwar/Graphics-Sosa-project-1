#define GLAD_GL_IMPLEMENTATION
#include "Renderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <map>

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

static GLuint kitchenTexture = 0;
static std::map<std::string, GLuint> pictureTextures;
static GLuint plateTexture = 0;
static GLuint foodTextures[FOOD_PICTURE_COUNT] = {};
static const char* foodFiles[FOOD_PICTURE_COUNT] = {
    "food-bread.png", "food-apple.png", "food-cheese.png"
};

struct KitchenDecoration {
    const char* filename;
    float position;  // Horizontal fraction of the window.
    float height;    // World units.
    GLuint texture = 0;
    int pixelWidth = 0;
    int pixelHeight = 0;
    float bottomPadding = 0.0f;
};

static KitchenDecoration decorations[] = {
    {"kitchen-pot.png",     0.10f, 0.22f},
    {"kitchen-herb.png",    0.30f, 0.23f},
    {"kitchen-jar.png",     0.50f, 0.22f},
    {"kitchen-toaster.png", 0.70f, 0.22f},
    {"kitchen-mug.png",     0.90f, 0.20f}
};

static GLuint loadKitchenTexture(const char* filename, int& width, int& height,
                                 float* bottomPadding = nullptr)
{
    int channels;
    unsigned char* pixels = stbi_load(filename, &width, &height,
                                      &channels, STBI_rgb_alpha);
    if (!pixels) {
        std::cerr << "Could not load " << filename << ": "
                  << stbi_failure_reason() << '\n';
        return 0;
    }

    // Ignore transparent padding when aligning decorations to the shelf.
    if (bottomPadding) {
        int bottom = height - 1;
        bool found = false;
        for (; bottom >= 0; --bottom) {
            for (int x = 0; x < width; ++x) {
                if (pixels[(bottom * width + x) * 4 + 3] > 16) {
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
        *bottomPadding = float(height - bottom - 1) / float(height);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

bool loadKitchenBackground()
{
    if (kitchenTexture != 0) return true;
    int width, height;
    kitchenTexture = loadKitchenTexture(GAME_ASSET_DIR "/kitchen-background.png", width, height);
    if (kitchenTexture == 0) return false;

    for (KitchenDecoration& decoration : decorations) {
        std::string filename = std::string(GAME_ASSET_DIR) + "/" + decoration.filename;
        decoration.texture = loadKitchenTexture(filename.c_str(),
                                                 decoration.pixelWidth, decoration.pixelHeight,
                                                 &decoration.bottomPadding);
        if (decoration.texture == 0) {
            releaseKitchenBackground();
            return false;
        }
    }
    plateTexture = loadKitchenTexture(GAME_ASSET_DIR "/food-plate.png", width, height);
    if (plateTexture == 0) {
        releaseKitchenBackground();
        return false;
    }
    for (int food = 0; food < FOOD_PICTURE_COUNT; ++food) {
        std::string filename = std::string(GAME_ASSET_DIR) + "/" + foodFiles[food];
        foodTextures[food] = loadKitchenTexture(filename.c_str(), width, height);
        if (foodTextures[food] == 0) {
            releaseKitchenBackground();
            return false;
        }
    }
    auto loadPicture = [&](const std::string& name) {
        std::string filename = std::string(GAME_ASSET_DIR) + "/" + name + ".png";
        GLuint texture = loadKitchenTexture(filename.c_str(), width, height);
        if (texture == 0) {
            releaseKitchenBackground();
            return false;
        }
        pictureTextures[name] = texture;
        return true;
    };
    for (const auto& group : {std::make_pair("intro-swatter-", 8), {"intro-fly-", 4},
                              {"intro-heading-", 5}, {"intro-dialogue-", 5}, {"intro-skip-", 2}}) {
        for (int frame = 0; frame < group.second; ++frame) {
            if (!loadPicture(std::string(group.first) + std::to_string(frame))) return false;
        }
    }
    for (const char* name : {"intro-steam", "intro-flash", "intro-progress"}) {
        if (!loadPicture(name)) return false;
    }
    for (int type = 0; type < 3; ++type) {
        for (int pose : {0, 2}) {
            // The sticky sheet has one static image.
            if (type == 2 && pose != 0) continue;
            int frameCount = type == 2 ? 1 : 8;
            for (int frame = 0; frame < frameCount; ++frame) {
                std::string name = "player-" + std::to_string(type) + "-"
                                 + std::to_string(pose) + "-" + std::to_string(frame);
                if (!loadPicture(name)) return false;
            }
        }
    }
    for (int type = 0; type < 3; ++type) {
        for (int frame = 0; frame < 5; ++frame) {
            if (!loadPicture("enemy-" + std::to_string(type) + "-" + std::to_string(frame))) return false;
        }
    }
    for (const char* name : {"projectile-spray", "projectile-enemy-0", "projectile-enemy-1"}) {
        if (!loadPicture(name)) return false;
    }
    if (!loadPicture("font")) return false;
    glBindTexture(GL_TEXTURE_2D, pictureTextures["font"]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    for (const char* name : {"menu-main", "menu-pause", "menu-win", "menu-lose"}) {
        if (!loadPicture(name)) return false;
    }
    for (const char* label : {"START_GAME", "EXIT_GAME", "CONTINUE", "RESTART", "MAIN_MENU", "PAUSE"}) {
        for (int hover = 0; hover < 2; ++hover) {
            if (!loadPicture(std::string("button-") + label + "-" + std::to_string(hover))) return false;
        }
    }
    for (const char* name : {"ui-fill", "hud-bar", "hud-coins", "icon-insect"}) {
        if (!loadPicture(name)) return false;
    }
    for (int state = 0; state < 4; ++state) {
        if (!loadPicture("hud-card-" + std::to_string(state))) return false;
    }
    for (const char* icon : {"coin", "swatter", "spray", "sticky"}) {
        for (int gray = 0; gray < 2; ++gray) {
            if (!loadPicture(std::string("icon-") + icon + "-" + std::to_string(gray))) return false;
        }
    }
    for (int wave = 1; wave <= 3; ++wave) {
        if (!loadPicture("wave-" + std::to_string(wave))) return false;
    }
    return true;
}

void releaseKitchenBackground()
{
    for (auto& picture : pictureTextures) {
        glDeleteTextures(1, &picture.second);
    }
    pictureTextures.clear();
    glDeleteTextures(1, &kitchenTexture);
    kitchenTexture = 0;
    glDeleteTextures(1, &plateTexture);
    plateTexture = 0;
    for (GLuint& texture : foodTextures) {
        glDeleteTextures(1, &texture);
        texture = 0;
    }
    for (KitchenDecoration& decoration : decorations) {
        glDeleteTextures(1, &decoration.texture);
        decoration.texture = 0;
    }
}

// Source image bands: mint wall, clear countertop, then cabinet fronts.
// Draw those bands at the same world heights as the playable counter.
static float counterHalfWidth = 1.0f;
static constexpr float WALL_TEXTURE_END = 0.213f;
static constexpr float COUNTER_TEXTURE_END = 0.803f;

static void drawKitchenImage(GLint transformLocation, GLuint texture,
                             float x, float y, float width, float height,
                             float u, float v, float uvWidth, float uvHeight)
{
    if (texture == 0) return;
    GLint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    GLint useTexture = glGetUniformLocation(program, "uUseTexture");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(program, "uTexture"), 0);
    glUniform4f(glGetUniformLocation(program, "uTextureRect"), u, v, uvWidth, uvHeight);
    glUniform1i(useTexture, GL_TRUE);
    drawRectangle(transformLocation, x, y, width, height);
    glUniform1i(useTexture, GL_FALSE);
}

void drawPictureTinted(GLint transformLocation, const std::string& name,
                       float x, float y, float width, float height)
{
    auto image = pictureTextures.find(name);
    if (image != pictureTextures.end()) {
        drawKitchenImage(transformLocation, image->second, x, y, width, height, 0, 0, 1, 1);
    }
}

void drawFontGlyph(GLint transformLocation, int character, float x, float y, float cellSize)
{
    auto font = pictureTextures.find("font");
    if (font == pictureTextures.end() || character < 0 || character >= 128 || cellSize <= 0) return;

    // Each atlas cell is 64px square; the glyph occupies its inner 40x56px.
    float u = ((character % 16) * 64 + 12) / 1024.0f;
    float v = ((character / 16) * 64 + 4) / 512.0f;
    drawKitchenImage(transformLocation, font->second, x + 2.5f * cellSize, y - 3.5f * cellSize,
                     5 * cellSize, 7 * cellSize, u, v, 40.0f / 1024, 56.0f / 512);
}

void drawIntroImage(GLint transformLocation, GLint colorLocation, const std::string& name,
                    float x, float y, float width, float height)
{
    auto picture = pictureTextures.find(name);
    if (picture == pictureTextures.end()) return;
    glUniform3f(colorLocation, 1, 1, 1);
    drawKitchenImage(transformLocation, picture->second, x, y, width, height, 0, 0, 1, 1);
}

void drawCharacterImage(GLint transformLocation, GLint colorLocation, const std::string& name,
                        float x, float y, float damage)
{
    GLint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    GLint tint = glGetUniformLocation(program, "uDamageTint");
    glUniform1f(tint, glm::clamp(damage, 0.0f, 1.0f) * 0.75f);
    drawIntroImage(transformLocation, colorLocation, name, x, y, 0.40f, 0.40f);
    glUniform1f(tint, 0);
}

void drawKitchenBackground(GLint transformLocation, GLint colorLocation,
                           const glm::mat4& projection)
{
    counterHalfWidth = 1.0f / projection[0][0];
    float halfHeight = 1.0f / projection[1][1];
    float width = counterHalfWidth * 2.0f;
    glUniform3f(colorLocation, 1.0f, 1.0f, 1.0f);

    // Shared boundaries prevent the image and game from forming separate panels.
    drawKitchenImage(transformLocation, kitchenTexture, 0.0f, (halfHeight + 0.62f) * 0.5f,
                     width, halfHeight - 0.62f, 0, 0, 1, WALL_TEXTURE_END);
    drawKitchenImage(transformLocation, kitchenTexture, 0.0f, -0.09f,
                     width, 1.42f, 0, WALL_TEXTURE_END, 1, COUNTER_TEXTURE_END - WALL_TEXTURE_END);
    drawKitchenImage(transformLocation, kitchenTexture, 0.0f, (-halfHeight - 0.80f) * 0.5f,
                     width, halfHeight - 0.80f, 0, COUNTER_TEXTURE_END, 1, 1.0f - COUNTER_TEXTURE_END);
}

void drawKitchenDecorations(GLint transformLocation, GLint colorLocation,
                            const glm::mat4& projection)
{
    float halfWidth = 1.0f / projection[0][0];
    float halfHeight = 1.0f / projection[1][1];
    glUniform3f(colorLocation, 1.0f, 1.0f, 1.0f);
    for (const KitchenDecoration& decoration : decorations) {
        if (decoration.texture == 0) continue;
        float x = -halfWidth + decoration.position * halfWidth * 2.0f;
        float height = decoration.height;
        float width = height * float(decoration.pixelWidth) / float(decoration.pixelHeight);
        float shelfY = -halfHeight + 0.015f;
        float y = shelfY + height * 0.5f - height * decoration.bottomPadding;
        drawKitchenImage(transformLocation, decoration.texture,
                         x, y, width, height, 0, 0, 1, 1);
    }
}

void drawFoodPlate(GLint transformLocation, GLint colorLocation, FoodPicture food,
                   float x, float y, float size)
{
    if (food < 0 || food >= FOOD_PICTURE_COUNT || size <= 0.0f) return;
    glUniform3f(colorLocation, 1.0f, 1.0f, 1.0f);
    drawKitchenImage(transformLocation, plateTexture, x, y, size, size, 0, 0, 1, 1);
    drawKitchenImage(transformLocation, foodTextures[food], x, y, size, size, 0, 0, 1, 1);
}

void drawKitchenCounter(GLint transformLocation,
                        float x, float y, float width, float height)
{
    // Sample the same countertop pixels beneath each lane; the current color
    // supplies a subtle hover/selection tint without hiding the image.
    float u = (x - width * 0.5f + counterHalfWidth) / (2.0f * counterHalfWidth);
    float v = WALL_TEXTURE_END + (0.62f - y - height * 0.5f) / 1.42f
              * (COUNTER_TEXTURE_END - WALL_TEXTURE_END);
    drawKitchenImage(transformLocation, kitchenTexture, x, y, width, height,
                     u, v, width / (2.0f * counterHalfWidth), height / 1.42f * (COUNTER_TEXTURE_END - WALL_TEXTURE_END));
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
