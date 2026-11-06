#include "TextRenderer.h"
#include "Renderer.h"
#include <cctype>

// Runtime text samples the font image; only the offline exporter builds glyphs.
void drawText(GLint transformLocation, const std::string& text, float x, float y, float cellSize)
{
    if (cellSize <= 0)
        return;
    for (unsigned char character : text)
    {
        int glyph = std::toupper(character);
        if (glyph != ' ')
            drawFontGlyph(transformLocation, glyph, x, y, cellSize);
        x += 6 * cellSize;
    }
}

void drawNumber(GLint transformLocation, int number, float x, float y, float cellSize)
{
    if (number >= 0)
        drawText(transformLocation, std::to_string(number), x, y, cellSize);
}
