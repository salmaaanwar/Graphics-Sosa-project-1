#include "ExampleScene.h"
#include "Renderer.h"
#include "TextRenderer.h"

//renderer usage examples
void drawExampleScene(
    GLint transformLocation,
    GLint colorLocation,
    int circleVertexStart,
    int circleVertexCount,
    int triangleVertexStart
){
    //red rectangle: center, full dimentions, rotation in degrees
    glUniform3f(colorLocation, 1.0f, 0.2f, 0.2f);
    drawRectangle(
        transformLocation, 
        -0.6f, 0.0f, 
        0.4f, 0.25f, 
        15.0f
    );

    //blue circle: center and radius
    glUniform3f(colorLocation, 0.2f, 0.5f, 1.0f);
    drawCircle(
        transformLocation,
        circleVertexStart, circleVertexCount,
        0.0f, 0.0f,
        0.15f
    );

    //green triangle: local origin, dimensions, default zero rotation
    glUniform3f(colorLocation, 0.2f, 0.8f, 0.3f);
    drawTriangle(
        transformLocation,
        triangleVertexStart,
        0.6f, 0.0f,
        0.35f, 0.35f
    );

    //START 125 white text
    glUniform3f(colorLocation, 1.0f, 1.0f, 1.0f);
    drawText(transformLocation, 
        "START 125", 
        -0.8f, 0.8f, 
        0.015f
    );
}