#pragma once
#include "../MathUtils.h"
#include <string>
#include <memory>

class TextRenderer;
class RectRenderer;
class SpriteRenderer;
class Texture;

class WarningPopup {
public:
    WarningPopup();
    ~WarningPopup();

    void Show(const std::string& message, float duration = 3.0f);
    void Update(float deltaTime);
    void Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer, SpriteRenderer* spriteRenderer);
    bool IsVisible() const { return mIsVisible; }

private:
    std::string mMessage;
    float mDuration;
    float mTimer;
    bool mIsVisible;
    std::unique_ptr<Texture> mBackgroundTexture;
};
