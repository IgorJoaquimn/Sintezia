#pragma once
#include <memory>
#include <vector>
#include <string>
#include "../MathUtils.h"

class Texture;
class SpriteRenderer;
class TextRenderer;

class ControlsUI {
public:
    ControlsUI();
    ~ControlsUI();

    void LoadTextures();
    void Draw(SpriteRenderer* spriteRenderer, TextRenderer* textRenderer);

private:
    std::shared_ptr<Texture> mKeyW;
    std::shared_ptr<Texture> mKeyA;
    std::shared_ptr<Texture> mKeyS;
    std::shared_ptr<Texture> mKeyD;
    std::shared_ptr<Texture> mKeyI;
    std::shared_ptr<Texture> mKeyZ;
};
