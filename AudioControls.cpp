#include "UI.h"
#include "Audio.h"
#include "Renderer.h"
#include "TextRenderer.h"
#include <cmath>

namespace {
constexpr float SLIDER_LEFT = -0.15f, SLIDER_RIGHT = 0.37f;
constexpr float ROW_Y[] = {-0.65f, -0.81f};
// Tab order: music slider, music mute, effects slider, effects mute.
int focusedControl = -1;
int draggedRow = -1;
int controlsState = -1;

bool controlsVisible() {
    return getGameState() == STATE_MAIN_MENU || getGameState() == STATE_PAUSED;
}

void resetControlsOnStateChange() {
    if (controlsState != getGameState()) {
        focusedControl = -1;
        draggedRow = -1;
        controlsState = getGameState();
    }
}

AudioBus rowBus(int row) { return row == 0 ? AudioBus::Music : AudioBus::Effects; }

void setVolumeFromMouse(int row, float x) {
    setAudioVolume(rowBus(row), (x - SLIDER_LEFT) / (SLIDER_RIGHT - SLIDER_LEFT));
}

void drawControlRect(GLint transformLocation, GLint colorLocation,
                     float x, float y, float width, float height,
                     float red, float green, float blue) {
    glUniform3f(colorLocation, red, green, blue);
    drawPictureTinted(transformLocation, "ui-fill", x, y, width, height);
}
}

void drawAudioControls(GLint transformLocation, GLint colorLocation) {
    // Keep input state changes out of rendering; tests also render without input.
    drawControlRect(transformLocation, colorLocation, 0, -0.715f, 1.68f, 0.39f, 0.13f, 0.19f, 0.18f);
    for (int row = 0; row < 2; ++row) {
        const float y = ROW_Y[row];
        const float volume = getAudioVolume(rowBus(row));
        const float sliderWidth = SLIDER_RIGHT - SLIDER_LEFT;
        glUniform3f(colorLocation, 1, 0.95f, 0.82f);
        drawText(transformLocation, row == 0 ? "MUSIC" : "SOUND EFFECTS", -0.76f, y + 0.018f, 0.006f);

        if (controlsState == getGameState() && focusedControl == row * 2) {
            drawControlRect(transformLocation, colorLocation, (SLIDER_LEFT + SLIDER_RIGHT) / 2,
                            y, sliderWidth + 0.055f, 0.085f, 1, 0.85f, 0.43f);
        }
        drawControlRect(transformLocation, colorLocation, (SLIDER_LEFT + SLIDER_RIGHT) / 2,
                        y, sliderWidth, 0.035f, 0.30f, 0.39f, 0.35f);
        if (volume > 0) {
            drawControlRect(transformLocation, colorLocation, SLIDER_LEFT + sliderWidth * volume / 2,
                            y, sliderWidth * volume, 0.035f, 0.46f, 0.79f, 0.66f);
        }
        drawControlRect(transformLocation, colorLocation, SLIDER_LEFT + sliderWidth * volume,
                        y, 0.025f, 0.065f, 1, 0.94f, 0.77f);

        glUniform3f(colorLocation, 1, 0.95f, 0.82f);
        const auto percentage = std::to_string(static_cast<int>(std::lround(volume * 100))) + "%";
        drawText(transformLocation, percentage, 0.415f, y + 0.018f, 0.005f);

        const bool muteFocused = controlsState == getGameState() && focusedControl == row * 2 + 1;
        drawControlRect(transformLocation, colorLocation, 0.665f, y, 0.24f, 0.10f,
                        muteFocused ? 0.64f : 0.29f, muteFocused ? 0.49f : 0.38f, 0.30f);
        glUniform3f(colorLocation, 1, 0.95f, 0.82f);
        drawCenteredText(transformLocation, volume == 0 ? "UNMUTE" : "MUTE", 0.665f, y, 0.005f);
    }
    glUniform3f(colorLocation, 1, 1, 1);
}

bool handleAudioControlsMouse(float x, float y, int action) {
    resetControlsOnStateChange();
    if (action == GLFW_RELEASE) {
        const bool handled = draggedRow >= 0;
        draggedRow = -1;
        return handled;
    }
    if (!controlsVisible() || action != GLFW_PRESS) return false;

    for (int row = 0; row < 2; ++row) {
        if (std::fabs(y - ROW_Y[row]) > 0.065f) continue;
        if (x >= SLIDER_LEFT - 0.03f && x <= SLIDER_RIGHT + 0.03f) {
            focusedControl = row * 2;
            draggedRow = row;
            setVolumeFromMouse(row, x);
            return true;
        }
        if (x >= 0.545f && x <= 0.785f) {
            focusedControl = row * 2 + 1;
            toggleAudioMute(rowBus(row));
            playSound(SoundId::Button);
            return true;
        }
    }
    focusedControl = -1;
    return false;
}

void updateAudioControlsMouse(float x, float, bool leftDown) {
    resetControlsOnStateChange();
    if (!controlsVisible() || !leftDown) {
        draggedRow = -1;
        return;
    }
    if (draggedRow >= 0) setVolumeFromMouse(draggedRow, x);
}

bool handleAudioControlsKey(int key, int action) {
    resetControlsOnStateChange();
    if (!controlsVisible() || (action != GLFW_PRESS && action != GLFW_REPEAT)) return false;
    if (key == GLFW_KEY_TAB) {
        if (action == GLFW_PRESS) focusedControl = (focusedControl + 1) % 4;
        return true;
    }
    if (focusedControl < 0) return false;

    const auto selectedBus = rowBus(focusedControl / 2);
    if (focusedControl % 2 == 0) {
        if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) {
            const float adjustment = key == GLFW_KEY_RIGHT ? 0.05f : -0.05f;
            setAudioVolume(selectedBus, getAudioVolume(selectedBus) + adjustment);
            return true;
        }
        if (key == GLFW_KEY_HOME || key == GLFW_KEY_END) {
            setAudioVolume(selectedBus, key == GLFW_KEY_HOME ? 0 : 1);
            return true;
        }
    } else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
        if (action == GLFW_PRESS) {
            toggleAudioMute(selectedBus);
            playSound(SoundId::Button);
        }
        return true;
    }
    return false;
}
