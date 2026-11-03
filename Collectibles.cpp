#include "Collectibles.h"
#include "Player.h"
#include "Deployment.h"
#include "Renderer.h"
#include <vector>
#include <cmath>
struct Drop { float x, y; };
static std::vector<Drop> drops;
void dropCollectible(float x, float y) { drops.push_back({x,y}); }
void resetCollectibles() { drops.clear(); }
void updateCollectibles() {
 for (int i=(int)drops.size()-1;i>=0;--i) for (const Player& p:getPlayers())
  if (std::abs(p.x-drops[i].x)<0.09f && std::abs(p.y-drops[i].y)<0.10f) { setCollectibles(getCollectibles()+1); drops.erase(drops.begin()+i); break; }
}
void drawCollectibles(GLint t, GLint c, int first, int count) {
 for (const Drop& d:drops) { glUniform3f(c,0.72f,0.38f,0.08f); drawCircle(t,first,count,d.x,d.y,0.025f); glUniform3f(c,1.0f,0.82f,0.25f); drawCircle(t,first,count,d.x,d.y,0.016f); }
}
