#include "GameOverUI.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include <SDL.h>

GameOverUI::GameOverUI() {
}

GameOverUI::~GameOverUI() {
}

void GameOverUI::Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer, SpriteRenderer* spriteRenderer) {
    if (!textRenderer || !rectRenderer) return;

    float windowWidth = (float)textRenderer->GetWindowWidth();
    float windowHeight = (float)textRenderer->GetWindowHeight();

    if (windowWidth <= 0) windowWidth = 1200.0f;
    if (windowHeight <= 0) windowHeight = 800.0f;

    // Draw semi-transparent black background
    rectRenderer->RenderRect(0, 0, windowWidth, windowHeight, Vector3(0.0f, 0.0f, 0.0f), 0.8f);

    SDL_Log("Drawing Game Over UI");

    // Draw "GAME OVER" text
    std::string title = "GAME OVER";
    float titleScale = 1.5f;
    float titleWidth = textRenderer->GetTextWidth(title, titleScale);
    float titleX = (windowWidth - titleWidth) / 2.0f;
    float titleY = windowHeight / 2.0f - 50.0f;

    textRenderer->SetTextColor(1.0f, 0.0f, 0.0f); // Red
    textRenderer->RenderText(title, titleX, titleY, titleScale);

    // Draw instructions
    std::string subtitle = "Pressione R para Reiniciar ou ESC para Sair";
    float subScale = 0.6f;
    float subWidth = textRenderer->GetTextWidth(subtitle, subScale);
    float subX = (windowWidth - subWidth) / 2.0f;
    float subY = titleY + 80.0f;

    textRenderer->SetTextColor(1.0f, 1.0f, 1.0f); // White
    textRenderer->RenderText(subtitle, subX, subY, subScale);
}
