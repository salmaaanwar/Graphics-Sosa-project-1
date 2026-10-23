#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

class Environment {
public:
    static const int LANE_COUNT = 4;

    void update(float dt);
    void render(GLint transformLocation, GLint colorLocation,
                int circleVertexStart, int circleVertexCount, int triangleVertexStart);
    void handleMouseMove(float mouseX, float mouseY);
    void handleMouseButton(int button, int action, float mouseX, float mouseY);

    int getLaneCount() const;
    float getLaneCenterY(int i) const;
    float getLaneTop(int i) const;
    float getLaneBottom(int i) const;
    float getLaneStartX() const;
    float getLaneEndX() const;
    float getHomeBaseRightEdgeX() const;
    int getLaneAtPoint(float x, float y) const;
    bool isLaneSelected(int i) const;
    std::vector<int> getSelectedLanes() const;
    void clearSelection();
    void reset();

private:
    float mouseX = -99.0f;
    float mouseY = -99.0f;
    bool leftButtonDown = false;
    float elapsed = 0.0f;
    int selected[LANE_COUNT] = { false, false, false, false };
};
