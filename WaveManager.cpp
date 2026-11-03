#include "WaveManager.h"
#include "Enemy.h"
#include "Environment.h"
#include "Game.h"
#include "Renderer.h"
#include <cstdlib>
static int wave=0, spawned=0;
static float timer=0;
static int waveSize[]={3,5,10};
void resetWaves(){wave=0;spawned=0;timer=0;}
void updateWaves(float dt){
 if(!isGameplayRunning()) return;
 if(wave>=3) return;
 if(spawned==waveSize[wave]) { if(countEnemies()==0){++wave;spawned=0;timer=0;if(wave==3)finishGame(true);} return; }
 timer-=dt; if(timer>0)return;
 int type=ENEMY_EASY;
 if(wave==1 && spawned%2)type=ENEMY_MEDIUM;
 if(wave==2) type=spawned%3==0?ENEMY_HARD:(spawned%2?ENEMY_MEDIUM:ENEMY_EASY);
 int lane=std::rand()%getEnvironment().getLaneCount();
 spawnEnemy(type,lane);++spawned;timer=1.2f;
}
void drawWaveStatus(GLint t,GLint c){(void)t;(void)c;}
