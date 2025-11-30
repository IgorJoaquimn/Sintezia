// filepath: src/UI/HealthBar.hpp
#pragma once

#include <memory>
#include "../MathUtils.h"

class HealthComponent;
class TextRenderer;
class RectRenderer;

class HealthBar {
public:
    HealthBar(HealthComponent* health, float x = 10.0f, float y = 10.0f, float width = 200.0f, float height = 20.0f);

    void Update(float deltaTime);
    void Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer);

    void SetPosition(float x, float y) { mX = x; mY = y; }
    void SetSize(float width, float height) { mW = width; mH = height; }

private:
    HealthComponent* mHealth;
    float mX;
    float mY;
    float mW;
    float mH;
};

