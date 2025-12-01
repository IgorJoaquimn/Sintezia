#pragma once
#include "../MathUtils.h"
#include <memory>

class TextRenderer;
class RectRenderer;
class SpriteRenderer;

class GameOverUI {
public:
    GameOverUI();
    ~GameOverUI();

    void Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer, SpriteRenderer* spriteRenderer);
};
