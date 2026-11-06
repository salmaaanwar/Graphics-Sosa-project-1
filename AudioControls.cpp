#include "UI.h"
#include "Audio.h"
#include "Renderer.h"
#include "TextRenderer.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float LEFT = -0.15f, RIGHT = 0.37f;
constexpr float ROW_Y[] = {-0.65f, -0.81f};
int focus = -1; // Music slider/mute, effects slider/mute.
int dragging = -1;
int controlsState = -1;
bool visible() { return getGameState() == STATE_MAIN_MENU || getGameState() == STATE_PAUSED; }
void synchronize() {
    if (controlsState != getGameState()) {
        focus = dragging = -1;
        controlsState = getGameState();
    }
}
AudioBus bus(int row) { return row == 0 ? AudioBus::Music : AudioBus::Effects; }
void slide(int row, float x) { setAudioVolume(bus(row), (x - LEFT) / (RIGHT - LEFT)); }
void rect(GLint t, GLint c, float x, float y, float w, float h, float r, float g, float b) {
    glUniform3f(c, r, g, b);
    drawPictureTinted(t, "ui-fill", x, y, w, h);
}
}
void drawAudioControls(GLint t, GLint c) {
    // Input updates own focus/dragging; rendering has no audio side effects.
    rect(t,c,0,-0.715f,1.68f,0.39f,0.13f,0.19f,0.18f);
    for (int row = 0; row < 2; ++row) {
        float y = ROW_Y[row], volume = getAudioVolume(bus(row));
        glUniform3f(c,1,0.95f,0.82f);
        drawText(t,row == 0 ? "MUSIC" : "SOUND EFFECTS",-0.76f,y+0.018f,0.006f);
        if (controlsState == getGameState() && focus == row*2)
            rect(t,c,(LEFT+RIGHT)/2,y,RIGHT-LEFT+0.055f,0.085f,1,0.85f,0.43f);
        rect(t,c,(LEFT+RIGHT)/2,y,RIGHT-LEFT,0.035f,0.30f,0.39f,0.35f);
        if (volume > 0)
            rect(t,c,LEFT+(RIGHT-LEFT)*volume/2,y,(RIGHT-LEFT)*volume,0.035f,0.46f,0.79f,0.66f);
        rect(t,c,LEFT+(RIGHT-LEFT)*volume,y,0.025f,0.065f,1,0.94f,0.77f);
        glUniform3f(c,1,0.95f,0.82f);
        drawText(t,std::to_string(static_cast<int>(std::lround(volume*100)))+"%",0.415f,y+0.018f,0.005f);
        bool focused = controlsState == getGameState() && focus == row*2+1;
        rect(t,c,0.665f,y,0.24f,0.10f,focused ? 0.64f : 0.29f,focused ? 0.49f : 0.38f,0.30f);
        glUniform3f(c,1,0.95f,0.82f);
        drawCenteredText(t,volume == 0 ? "UNMUTE" : "MUTE",0.665f,y,0.005f);
    }
    glUniform3f(c,1,1,1);
}
bool handleAudioControlsMouse(float x, float y, int action) {
    synchronize();
    if (action == GLFW_RELEASE) { bool handled = dragging >= 0; dragging = -1; return handled; }
    if (!visible() || action != GLFW_PRESS) return false;
    for (int row = 0; row < 2; ++row) {
        if (std::fabs(y-ROW_Y[row]) > 0.065f) continue;
        if (x >= LEFT-0.03f && x <= RIGHT+0.03f) {
            focus = row*2; dragging = row; slide(row,x); return true;
        }
        if (x >= 0.545f && x <= 0.785f) {
            focus = row*2+1; toggleAudioMute(bus(row)); playSound(SoundId::Button); return true;
        }
    }
    focus = -1;
    return false;
}
void updateAudioControlsMouse(float x, float, bool leftDown) {
    synchronize();
    if (!visible() || !leftDown) { dragging = -1; return; }
    if (dragging >= 0) slide(dragging,x);
}
bool handleAudioControlsKey(int key, int action) {
    synchronize();
    if (!visible() || (action != GLFW_PRESS && action != GLFW_REPEAT)) return false;
    if (key == GLFW_KEY_TAB) {
        if (action == GLFW_PRESS) focus = (focus+1)%4;
        return true;
    }
    if (focus < 0) return false;
    const auto selected = bus(focus/2);
    if (focus%2 == 0) {
        if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) {
            setAudioVolume(selected,getAudioVolume(selected)+(key == GLFW_KEY_RIGHT ? 0.05f : -0.05f));
            return true;
        }
        if (key == GLFW_KEY_HOME || key == GLFW_KEY_END) {
            setAudioVolume(selected,key == GLFW_KEY_HOME ? 0 : 1); return true;
        }
    } else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
        if (action == GLFW_PRESS) { toggleAudioMute(selected); playSound(SoundId::Button); }
        return true;
    }
    return false;
}
