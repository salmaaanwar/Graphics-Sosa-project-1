#pragma once
#include <glad/gl.h>
#include <string>

//digit drawing
void drawDigit(
    GLint transformLocation, 
    int digit,
    float x,
    float y,
    float cellSize
);

//number drawing
void drawNumber(
    GLint transformLocation,
    int number, 
    float x,
    float y,
    float cellSize
);

//text drawing
void drawText(
    GLint transformlocation,
    const std::string& text,
    float x,
    float y,
    float cellSize
);