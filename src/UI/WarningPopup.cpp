#include "WarningPopup.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include "../Core/Texture/Texture.hpp"
#include <iostream>
#include <SDL.h>

WarningPopup::WarningPopup() : mDuration(0.0f), mTimer(0.0f), mIsVisible(false) {
    mBackgroundTexture = std::make_unique<Texture>();
    // Try loading from different paths
    if (!mBackgroundTexture->Load("assets/third_party/Ninja Adventure - Asset Pack/Ui/Dialog/DialogueBoxSimple.png")) {
        mBackgroundTexture->Load("../assets/third_party/Ninja Adventure - Asset Pack/Ui/Dialog/DialogueBoxSimple.png");
    }
}

WarningPopup::~WarningPopup() {
    if (mBackgroundTexture) {
        mBackgroundTexture->Unload();
    }
}

void WarningPopup::Show(const std::string& message, float duration) {
    mMessage = message;
    mDuration = duration;
    mTimer = 0.0f;
    mIsVisible = true;
}

void WarningPopup::Update(float deltaTime) {
    if (mIsVisible) {
        mTimer += deltaTime;
        if (mTimer >= mDuration) {
            mIsVisible = false;
        }
    }
}

void WarningPopup::Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer, SpriteRenderer* spriteRenderer) {
    if (!mIsVisible) return;

    float windowWidth = (float)textRenderer->GetWindowWidth();
    float windowHeight = (float)textRenderer->GetWindowHeight();

    // Fallback if window dimensions are zero
    if (windowWidth <= 0) windowWidth = 1200.0f;
    if (windowHeight <= 0) windowHeight = 800.0f;

    float boxWidth = 600.0f;
    float boxHeight = 150.0f;
    float x = (windowWidth - boxWidth) / 2.0f;
    float y = (windowHeight - boxHeight) / 2.0f;

    // Draw background
    if (mBackgroundTexture && spriteRenderer && mBackgroundTexture->GetWidth() > 0) {
         spriteRenderer->DrawSprite(mBackgroundTexture.get(), Vector2(x, y), Vector2(boxWidth, boxHeight));
    } else if (rectRenderer) {
         // Draw a red box if texture is missing, just to be visible for debug
         rectRenderer->RenderRect(x, y, boxWidth, boxHeight, Vector3(0.5f, 0.0f, 0.0f), 0.8f);
    }

    // Draw text
    if (textRenderer) {
        float scale = 0.6f;
        float textWidth = textRenderer->GetTextWidth(mMessage, scale);
        
        // Auto-scale to fit width
        float maxWidth = boxWidth - 60.0f; // 30px padding on each side
        if (textWidth > maxWidth) {
            scale *= (maxWidth / textWidth);
            textWidth = maxWidth;
        }

        float textX = x + (boxWidth - textWidth) / 2.0f;
        // Center vertically roughly
        float textY = y + (boxHeight / 2.0f) - (10.0f * scale); 
        
        textRenderer->SetTextColor(0.0f, 0.0f, 0.0f); // Black text
        textRenderer->RenderText(mMessage, textX, textY, scale);
        textRenderer->SetTextColor(1.0f, 1.0f, 1.0f); // Reset to white
    }
}
