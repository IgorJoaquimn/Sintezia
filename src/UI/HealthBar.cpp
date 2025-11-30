//
// Created by giova on 29/11/2025.
//// filepath: src/UI/HealthBar.cpp
#include "HealthBar.hpp"
#include "../Component/HealthComponent.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include <string>

HealthBar::HealthBar(HealthComponent* health, float x, float y, float width, float height)
    : mHealth(health)
    , mX(x)
    , mY(y)
    , mW(width)
    , mH(height)
{
}

void HealthBar::Update(float /*deltaTime*/)
{
    // Currently no smoothing; placeholder for future animations
}

void HealthBar::Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!mHealth || !rectRenderer) return;

    float current = mHealth->GetCurrentHealth();
    float max = mHealth->GetMaxHealth();
    float ratio = (max > 0.0f) ? (current / max) : 0.0f;
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    // Background box
    rectRenderer->RenderRect(mX, mY, mW, mH, Vector3(0.08f, 0.08f, 0.1f), 0.95f);

    // Inner padding
    const float pad = 2.0f;
    float innerX = mX + pad;
    float innerY = mY + pad;
    float innerW = std::max(0.0f, mW - pad * 2.0f);
    float innerH = std::max(0.0f, mH - pad * 2.0f);

    // Fill color: green when full, red when low
    Vector3 fillColor((1.0f - ratio), ratio, 0.0f);
    rectRenderer->RenderRect(innerX, innerY, innerW * ratio, innerH, fillColor, 1.0f);

    // Optional: draw remaining portion with darker overlay for contrast
    rectRenderer->RenderRect(innerX + innerW * ratio, innerY, innerW * (1.0f - ratio), innerH, Vector3(0.15f, 0.15f, 0.15f), 0.7f);

    // Draw text (current / max) centralizado dentro da barra
    if (textRenderer)
    {
        textRenderer->SetTextColor(1.0f, 1.0f, 1.0f); // branco
        std::string text = std::to_string(static_cast<int>(current)) + " / " + std::to_string(static_cast<int>(max));
        float textScale = 0.7f;
        Vector2 size = textRenderer->MeasureText(text, textScale);
        // Centraliza o texto dentro da barra
        float tx = mX + (mW - size.x) / 2.0f;
        float ty = mY + (mH - size.y) / 2.0f - 8.0f;
        textRenderer->RenderText(text, tx, ty, textScale);
    }
}
