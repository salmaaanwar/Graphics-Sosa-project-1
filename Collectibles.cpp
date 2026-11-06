#include "Collectibles.h"
#include "Deployment.h"
#include "Player.h"
#include "Renderer.h"
#include <cmath>
#include <vector>

struct CollectibleDrop
{
    float x;
    float y;
    int value;
};

static std::vector<CollectibleDrop> drops;

void resizeCollectiblePositions(float scale, float offset)
{
    for (CollectibleDrop& drop : drops) drop.x = drop.x * scale + offset;
}

void dropCollectible(float x, float y, int value)
{
    drops.push_back({x, y, value});
}

void resetCollectibles()
{
    drops.clear();
}

void updateCollectibles()
{
    // Walk backward so erasing a pickup leaves the remaining indices valid.
    for (int index = static_cast<int>(drops.size()) - 1; index >= 0; --index)
    {
        const CollectibleDrop& drop = drops[index];
        for (const Player& player : getPlayers())
        {
            const bool withinReach = std::abs(player.x - drop.x) < 0.09f &&
                                     std::abs(player.y - drop.y) < 0.10f;
            if (withinReach)
            {
                collectEnemyDrop(drop.value);
                drops.erase(drops.begin() + index);
                break;
            }
        }
    }
}

void drawCollectibles(GLint transformLocation, GLint colorLocation,
                      int, int)
{
    for (const CollectibleDrop& drop : drops)
    {
        drawIntroImage(transformLocation, colorLocation, "icon-coin-0", drop.x, drop.y, .0625f, .0625f);
    }
}
