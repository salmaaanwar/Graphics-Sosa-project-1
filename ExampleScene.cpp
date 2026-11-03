#include "ExampleScene.h"
#include "Renderer.h"
#include "TextRenderer.h"

void drawExampleScene(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart
){
    // Red rectangle with full width and height, rotated in degrees.
    glUniform3f(colorLocation, 1.0f, 0.2f, 0.2f);
    drawRectangle(
        transformLocation, 
        -0.6f, 0.0f, 
        0.4f, 0.25f, 
        15.0f
    );

    // Blue circle positioned by its center.
    glUniform3f(colorLocation, 0.2f, 0.5f, 1.0f);
    drawCircle(
        transformLocation,
        circleVertexStart, circleVertexCount,
        0.0f, 0.0f,
        0.15f
    );

    // Green triangle with the default rotation.
    glUniform3f(colorLocation, 0.2f, 0.8f, 0.3f);
    drawTriangle(
        transformLocation,
        triangleVertexStart,
        0.6f, 0.0f,
        0.35f, 0.35f
    );

    // White title text.
    glUniform3f(colorLocation, 1.0f, 1.0f, 1.0f);
    drawText(transformLocation, 
        "START 125", 
        -0.8f, 0.8f, 
        0.015f
    );
}