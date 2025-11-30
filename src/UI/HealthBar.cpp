//
// Created by giova on 29/11/2025.
//// filepath: src/UI/HealthBar.cpp
#include "HealthBar.hpp"
#include "../Component/HealthComponent.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include <string>

HealthBar::HealthBar(HealthComponent* health, HungerComponent* hunger, ThirstComponent* thirst, RectRenderer* rectRenderer, float x, float y, float width, float height)
    : mHealth(health)
    , mHunger(hunger)
    , mThirst(thirst)
    , mRectRenderer(rectRenderer)
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
    if (!mHealth || !mHunger || !mThirst || !rectRenderer) return;

    float currentHealth = mHealth->GetCurrentHealth();
    float maxHealth = mHealth->GetMaxHealth();
    float healthRatio = (maxHealth > 0.0f) ? (currentHealth / maxHealth) : 0.0f;

    float currentHunger = mHunger->GetCurrentHunger();
    float maxHunger = mHunger->GetMaxHunger();
    float hungerRatio = (maxHunger > 0.0f) ? (currentHunger / maxHunger) : 0.0f;

    float currentThirst = mThirst->GetCurrentThirst();
    float maxThirst = mThirst->GetMaxThirst();
    float thirstRatio = (maxThirst > 0.0f) ? (currentThirst / maxThirst) : 0.0f;

    // Clamp ratios
    if (healthRatio < 0.0f) healthRatio = 0.0f;
    if (healthRatio > 1.0f) healthRatio = 1.0f;
    if (hungerRatio < 0.0f) hungerRatio = 0.0f;
    if (hungerRatio > 1.0f) hungerRatio = 1.0f;
    if (thirstRatio < 0.0f) thirstRatio = 0.0f;
    if (thirstRatio > 1.0f) thirstRatio = 1.0f;

    // Calculate widths
    float totalW = mW;
    float thirstW = totalW * thirstRatio;
    float hungerW = totalW * hungerRatio;
    float damageW = totalW * (1.0f - healthRatio);

    // Calculate positions from Right to Left
    
    // Thirst (Right aligned)
    float thirstX = mX + mW - thirstW;
    // Clamp to bar bounds
    if (thirstX < mX) { thirstX = mX; thirstW = mW; }

    // Hunger (Left of Thirst)
    float hungerX = thirstX - hungerW;
    if (hungerX < mX) { 
        hungerW = thirstX - mX; // Remaining space
        hungerX = mX; 
    }

    // Damage (Left of Hunger)
    float damageX = hungerX - damageW;
    if (damageX < mX) {
        damageW = hungerX - mX; // Remaining space
        damageX = mX;
    }

    // Health (Left of Damage, fills remaining space from mX)
    float healthX = mX;
    float visibleHealthW = damageX - healthX;
    if (visibleHealthW < 0.0f) visibleHealthW = 0.0f;

    // Draw Background (Dark)
    // Draw a white border first (simulated by drawing a larger white rect behind)
    float border = 2.0f;
    float radius = 8.0f;
    rectRenderer->RenderRoundedRect(mX - border, mY - border, mW + border * 2, mH + border * 2, Vector3(1.0f, 1.0f, 1.0f), 1.0f, radius + 1.0f, false, 15);
    
    // Draw inner background
    rectRenderer->RenderRoundedRect(mX, mY, mW, mH, Vector3(0.1f, 0.1f, 0.1f), 1.0f, radius, false, 15);

    // Draw Health (Green)
    if (visibleHealthW > 0.0f)
    {
        // Round left corners (1 | 8 = 9)
        // If it fills the whole bar, round all (15)
        // If it's the only thing visible, round all.
        // But here we have other bars to the right.
        // So round Left corners.
        int corners = 9; // TL | BL
        rectRenderer->RenderRoundedRect(healthX, mY, visibleHealthW, mH, Vector3(0.5f, 0.8f, 0.2f), 1.0f, radius, false, corners);
    }

    // Draw Damage/Empty (Red)
    if (damageW > 0.0f)
    {
        // Usually square, unless it's at the ends (which it isn't, usually in middle)
        // If health is 0, it starts at left -> round left
        // If hunger/thirst is 0, it ends at right -> round right
        int corners = 0;
        if (visibleHealthW <= 0.0f) corners |= 9; // TL | BL
        if (hungerW <= 0.0f && thirstW <= 0.0f) corners |= 6; // TR | BR
        
        rectRenderer->RenderRoundedRect(damageX, mY, damageW, mH, Vector3(0.8f, 0.2f, 0.2f), 1.0f, radius, false, corners);
    }

    // Draw Hunger (Yellow)
    if (hungerW > 0.0f)
    {
        int corners = 0;
        if (visibleHealthW <= 0.0f && damageW <= 0.0f) corners |= 9; // TL | BL
        if (thirstW <= 0.0f) corners |= 6; // TR | BR
        
        rectRenderer->RenderRoundedRect(hungerX, mY, hungerW, mH, Vector3(0.9f, 0.7f, 0.1f), 1.0f, radius, true, corners);
    }

    // Draw Thirst (Light Blue)
    if (thirstW > 0.0f)
    {
        int corners = 6; // TR | BR
        if (visibleHealthW <= 0.0f && damageW <= 0.0f && hungerW <= 0.0f) corners |= 9; // TL | BL
        
        rectRenderer->RenderRoundedRect(thirstX, mY, thirstW, mH, Vector3(0.2f, 0.6f, 0.9f), 1.0f, radius, true, corners);
    }

    if (textRenderer)
    {
        float scale = 0.6f; // Slightly larger icons
        float yOffset = 15.0f; // Increased distance above the bar

        // Draw Heart Emoji if taking damage
        if (damageW > 0.0f)
        {
            // Center on damage bar
            float iconX = damageX + damageW / 2.0f;
            
            std::string heart = "\xE2\x9D\xA4"; // U+2764 (Heart without variation selector)
            Vector2 size = textRenderer->MeasureText(heart, scale);
            
            float tx = iconX - size.x / 2.0f;
            float ty = mY - yOffset; 

            textRenderer->RenderText(heart, tx, ty, scale);
        }

        // Draw Hunger Emoji (Chicken Leg)
        if (hungerW > 0.0f)
        {
            // Center on hunger bar
            float iconX = hungerX + hungerW / 2.0f;
            
            std::string food = "🍗"; // U+1F357
            Vector2 size = textRenderer->MeasureText(food, scale);
            
            float tx = iconX - size.x / 2.0f;
            float ty = mY - yOffset;

            textRenderer->RenderText(food, tx, ty, scale);
        }

        // Draw Thirst Emoji (Drop)
        if (thirstW > 0.0f)
        {
            // Center on thirst bar
            float iconX = thirstX + thirstW / 2.0f;
            
            std::string drop = "💧"; // U+1F4A7
            Vector2 size = textRenderer->MeasureText(drop, scale);
            
            float tx = iconX - size.x / 2.0f;
            float ty = mY - yOffset;

            textRenderer->RenderText(drop, tx, ty, scale);
        }
    }
}
