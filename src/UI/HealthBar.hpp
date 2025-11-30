// filepath: src/UI/HealthBar.hpp
#pragma once

#include <memory>
#include "../MathUtils.h"
#include "../Component/HungerComponent.hpp"
#include "../Component/ThirstComponent.hpp"

class HealthComponent;
class TextRenderer;
class RectRenderer;

class HealthBar {
public:
    HealthBar(HealthComponent* health, HungerComponent* hunger, ThirstComponent* thirst, RectRenderer* rectRenderer, float x = 10.0f, float y = 10.0f, float width = 200.0f, float height = 20.0f);

    void Update(float deltaTime);
    void Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer);

    void SetPosition(float x, float y) { mX = x; mY = y; }
    void SetSize(float width, float height) { mW = width; mH = height; }

private:
    HealthComponent* mHealth;
    HungerComponent* mHunger;
    ThirstComponent* mThirst;
    float mX;
    float mY;
    float mW;
    float mH;
    RectRenderer* mRectRenderer; // Add member variable for rectRenderer
};

