#pragma once
#include <glad/gl.h>
void dropCollectible(float x, float y);
void updateCollectibles();
void resetCollectibles();
void drawCollectibles(GLint transformLocation, GLint colorLocation, int circleVertexStart, int circleVertexCount);
