#pragma once
#include <glad/gl.h>

const float CUTSCENE_DURATION = 16.0f;

void resetCutscene();
void updateCutscene(float deltaTime);
float getCutsceneTime();
bool isCutsceneFinished();

void drawCutscene(GLint transformLocation, GLint colorLocation);
