#pragma once
#include "../MathUtils.h"
#include <memory>

class TextRenderer;
class RectRenderer;
class SpriteRenderer;

class VictoryUI {
public:
    VictoryUI();
    ~VictoryUI();

    void Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer, SpriteRenderer* spriteRenderer);
};
