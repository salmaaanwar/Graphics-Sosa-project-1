#pragma once
#include <glad/gl.h>
#include <string>

void drawDigit(
    GLint transformLocation, 
    int digit,
    float x,
    float y,
    float cellSize
);

// Negative numbers are not rendered.
void drawNumber(
    GLint transformLocation,
    int number, 
    float x,
    float y,
    float cellSize
);

// Text starts at its top-left corner; the atlas uses uppercase ASCII glyphs.
void drawText(
    GLint transformLocation,
    const std::string& text,
    float x,
    float y,
    float cellSize
);
