#include "UI.h"
#include "Audio.h"
#include "Renderer.h"
#include "TextRenderer.h"
#include <algorithm>
#include <cmath>

namespace
{
constexpr float LEFT = -0.15f, RIGHT = 0.23f;
constexpr float ROW_Y[] = {0.16f, -0.08f};
int focus = -1; // Music slider/mute, effects slider/mute.
int dragging = -1;
int controlsState = -1;
bool visible()
{
    return isSoundSettingsOpen();
}
void synchronize()
{
    if (controlsState != (visible() ? 1 : 0))
    {
        focus = dragging = -1;
        controlsState = visible() ? 1 : 0;
    }
}
AudioBus bus(int row)
{
    if (row == 0)
    {
        return AudioBus::Music;
    }
    return AudioBus::Effects;
}
void slide(int row, float x)
{
    setAudioVolume(bus(row), (x - LEFT) / (RIGHT - LEFT));
}
void rect(GLint t, GLint c, float x, float y, float w, float h, float r, float g, float b)
{
    glUniform3f(c, r, g, b);
    drawPictureTinted(t, "ui-fill", x, y, w, h);
}
} // namespace
void drawAudioControls(GLint t, GLint c)
{
    // Input updates own focus/dragging; rendering has no audio side effects.
    rect(t, c, 0, .04f, 1.38f, .48f, .11f, .16f, .23f);
    for (int row = 0; row < 2; ++row)
    {
        float y = ROW_Y[row], volume = getAudioVolume(bus(row));
        glUniform3f(c, 1, 0.95f, 0.82f);
        drawLabelImage(t, row == 0 ? "MUSIC" : "SOUND EFFECTS", row == 0 ? -.575f : -.48025f, y,
                       0.0045f);
        if (visible() && focus == row * 2)
            rect(t, c, (LEFT + RIGHT) / 2, y, RIGHT - LEFT + 0.055f, 0.085f, 1, 0.85f, 0.43f);
        rect(t, c, (LEFT + RIGHT) / 2, y, RIGHT - LEFT, 0.035f, .23f, .30f, .40f);
        if (volume > 0)
            rect(t, c, LEFT + (RIGHT - LEFT) * volume / 2, y, (RIGHT - LEFT) * volume, 0.035f, .25f,
                 .60f, .85f);
        rect(t, c, LEFT + (RIGHT - LEFT) * volume, y, 0.025f, 0.065f, 1, 0.94f, 0.77f);
        glUniform3f(c, 1, 0.95f, 0.82f);
        drawText(t, std::to_string(static_cast<int>(std::lround(volume * 100))) + "%", 0.28f,
                 y + 0.018f, 0.004f);
        bool focused = visible() && focus == row * 2 + 1;
        rect(t, c, 0.55f, y, 0.22f, 0.10f, focused ? 0.64f : 0.29f, focused ? 0.49f : 0.38f, 0.30f);
        glUniform3f(c, 1, 0.95f, 0.82f);
        drawLabelImage(t, volume == 0 ? "UNMUTE" : "MUTE", 0.55f, y, 0.005f);
    }
    glUniform3f(c, 1, 1, 1);
}
bool handleAudioControlsMouse(float x, float y, int action)
{
    synchronize();
    if (action == GLFW_RELEASE)
    {
        bool handled = dragging >= 0;
        dragging = -1;
        return handled;
    }
    if (!visible() || action != GLFW_PRESS)
        return false;
    for (int row = 0; row < 2; ++row)
    {
        if (std::fabs(y - ROW_Y[row]) > 0.065f)
            continue;
        if (x >= LEFT - 0.03f && x <= RIGHT + 0.03f)
        {
            focus = row * 2;
            dragging = row;
            slide(row, x);
            return true;
        }
        if (x >= 0.44f && x <= 0.66f)
        {
            focus = row * 2 + 1;
            toggleAudioMute(bus(row));
            playSound(SoundId::Button);
            return true;
        }
    }
    focus = -1;
    return false;
}
void updateAudioControlsMouse(float x, float, bool leftDown)
{
    synchronize();
    if (!visible() || !leftDown)
    {
        dragging = -1;
        return;
    }
    if (dragging >= 0)
        slide(dragging, x);
}
bool handleAudioControlsKey(int key, int action)
{
    synchronize();
    if (!visible() || (action != GLFW_PRESS && action != GLFW_REPEAT))
        return false;
    if (key == GLFW_KEY_TAB)
    {
        if (action == GLFW_PRESS)
            focus = (focus + 1) % 4;
        return true;
    }
    if (focus < 0)
        return false;
    AudioBus selected = bus(focus / 2);
    if (focus % 2 == 0)
    {
        if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT)
        {
            float change = -0.05f;
            if (key == GLFW_KEY_RIGHT)
            {
                change = 0.05f;
            }
            setAudioVolume(selected, getAudioVolume(selected) + change);
            return true;
        }
        if (key == GLFW_KEY_HOME || key == GLFW_KEY_END)
        {
            float volume = 1;
            if (key == GLFW_KEY_HOME)
            {
                volume = 0;
            }
            setAudioVolume(selected, volume);
            return true;
        }
    }
    else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
    {
        if (action == GLFW_PRESS)
        {
            toggleAudioMute(selected);
            playSound(SoundId::Button);
        }
        return true;
    }
    return false;
}
