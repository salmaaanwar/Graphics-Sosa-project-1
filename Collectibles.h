#pragma once
#include <glad/gl.h>
void dropCollectible(float x, float y, int value);
void updateCollectibles();
void resetCollectibles();
void resizeCollectiblePositions(float scale, float offset);
void drawCollectibles(GLint transformLocation, GLint colorLocation);
