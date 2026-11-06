// Offline source glyph artwork; excluded from the running game.
#include "TextRenderer.h"
#include "Renderer.h"
#include <string>

// A glyph uses a 5-by-7 grid. Draw a square for each filled cell.
static void drawPattern(
    GLint transformLocation,
    const char* const pattern[],
    float x,
    float y,
    float cellSize
) {
    for (int row = 0; row < 7; row++) {
        for (int column = 0; column < 5; column++) {
            if (pattern[row][column] == '1') {
                float cellX = x + (column + 0.5f) * cellSize;
                float cellY = y - (row + 0.5f) * cellSize;

                drawRectangle(
                    transformLocation,
                    cellX, cellY,
                    cellSize, cellSize
                );
            }
        }
    }
}

void drawDigit(
    GLint transformLocation,
    int digit,
    float x,
    float y,
    float cellSize
) {
    if (digit < 0 || digit > 9 || cellSize <= 0.0f) {
        return;
    }

    static const char* patterns[10][7] = {
        {"01110", "10001", "10001", "10001", "10001", "10001", "01110"}, // 0
        {"00100", "01100", "00100", "00100", "00100", "00100", "01110"}, // 1
        {"01110", "10001", "00001", "00010", "00100", "01000", "11111"}, // 2
        {"11110", "00001", "00001", "01110", "00001", "00001", "11110"}, // 3
        {"00010", "00110", "01010", "10010", "11111", "00010", "00010"}, // 4
        {"11111", "10000", "10000", "11110", "00001", "00001", "11110"}, // 5
        {"01110", "10000", "10000", "11110", "10001", "10001", "01110"}, // 6
        {"11111", "00001", "00010", "00100", "01000", "01000", "01000"}, // 7
        {"01110", "10001", "10001", "01110", "10001", "10001", "01110"}, // 8
        {"01110", "10001", "10001", "01111", "00001", "00001", "01110"}  // 9
    };

    drawPattern(
        transformLocation,
        patterns[digit],
        x, y, cellSize
    );
}

void drawNumber(
    GLint transformLocation,
    int number,
    float x,
    float y,
    float cellSize
) {
    if (number < 0 || cellSize <= 0.0f) {
        return;
    }

    std::string digits = std::to_string(number);

    for (char character : digits) {
        int digit = character - '0';

        drawDigit(transformLocation, digit, x, y, cellSize);

        x += 6.0f * cellSize;
    }
}

void drawText(
    GLint transformLocation,
    const std::string& text,
    float x,
    float y,
    float cellSize
) {
    if (cellSize <= 0.0f) {
        return;
    }

    static const char* letters[26][7] = {
        {"01110", "10001", "10001", "11111", "10001", "10001", "10001"}, // A
        {"11110", "10001", "10001", "11110", "10001", "10001", "11110"}, // B
        {"01111", "10000", "10000", "10000", "10000", "10000", "01111"}, // C
        {"11110", "10001", "10001", "10001", "10001", "10001", "11110"}, // D
        {"11111", "10000", "10000", "11110", "10000", "10000", "11111"}, // E
        {"11111", "10000", "10000", "11110", "10000", "10000", "10000"}, // F
        {"01111", "10000", "10000", "10111", "10001", "10001", "01111"}, // G
        {"10001", "10001", "10001", "11111", "10001", "10001", "10001"}, // H
        {"11111", "00100", "00100", "00100", "00100", "00100", "11111"}, // I
        {"00111", "00010", "00010", "00010", "10010", "10010", "01100"}, // J
        {"10001", "10010", "10100", "11000", "10100", "10010", "10001"}, // K
        {"10000", "10000", "10000", "10000", "10000", "10000", "11111"}, // L
        {"10001", "11011", "10101", "10101", "10001", "10001", "10001"}, // M
        {"10001", "11001", "10101", "10011", "10001", "10001", "10001"}, // N
        {"01110", "10001", "10001", "10001", "10001", "10001", "01110"}, // O
        {"11110", "10001", "10001", "11110", "10000", "10000", "10000"}, // P
        {"01110", "10001", "10001", "10001", "10101", "10010", "01101"}, // Q
        {"11110", "10001", "10001", "11110", "10100", "10010", "10001"}, // R
        {"01111", "10000", "10000", "01110", "00001", "00001", "11110"}, // S
        {"11111", "00100", "00100", "00100", "00100", "00100", "00100"}, // T
        {"10001", "10001", "10001", "10001", "10001", "10001", "01110"}, // U
        {"10001", "10001", "10001", "10001", "10001", "01010", "00100"}, // V
        {"10001", "10001", "10001", "10101", "10101", "10101", "01010"}, // W
        {"10001", "10001", "01010", "00100", "01010", "10001", "10001"}, // X
        {"10001", "10001", "01010", "00100", "00100", "00100", "00100"}, // Y
        {"11111", "00001", "00010", "00100", "01000", "10000", "11111"}  // Z
    };

    for (char character : text) {
        if (character >= 'A' && character <= 'Z') {
            int index = character - 'A';

            drawPattern(
                transformLocation,
                letters[index],
                x, y, cellSize
            );
        }
        else if (character >= '0' && character <= '9') {
            drawDigit(
                transformLocation,
                character - '0',
                x, y, cellSize
            );
        }

        x += 6.0f * cellSize;
    }
}