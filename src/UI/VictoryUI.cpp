#include "VictoryUI.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include <SDL.h>
#include <string>

VictoryUI::VictoryUI() {
}

VictoryUI::~VictoryUI() {
}

void VictoryUI::Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer, SpriteRenderer* spriteRenderer) {
    if (!textRenderer || !rectRenderer) return;

    float windowWidth = (float)textRenderer->GetWindowWidth();
    float windowHeight = (float)textRenderer->GetWindowHeight();

    if (windowWidth <= 0) windowWidth = 1200.0f;
    if (windowHeight <= 0) windowHeight = 800.0f;

    // Draw semi-transparent black background (maybe slightly blue/gold for victory?)
    rectRenderer->RenderRect(0, 0, windowWidth, windowHeight, Vector3(0.0f, 0.0f, 0.2f), 0.8f);

    // Draw "VICTORY!" text
    std::string title = "VITORIA!";
    float titleScale = 1.5f;
    float titleWidth = textRenderer->GetTextWidth(title, titleScale);
    float titleX = (windowWidth - titleWidth) / 2.0f;
    float titleY = windowHeight / 2.0f - 50.0f;

    textRenderer->SetTextColor(0.0f, 1.0f, 0.0f); // Green
    textRenderer->RenderText(title, titleX, titleY, titleScale);

    // Draw message
    std::string message = "Voce escapou da ilha!";
    float msgScale = 0.8f;
    float msgWidth = textRenderer->GetTextWidth(message, msgScale);
    float msgX = (windowWidth - msgWidth) / 2.0f;
    float msgY = titleY + 60.0f;
    
    textRenderer->SetTextColor(1.0f, 1.0f, 0.0f); // Yellow
    textRenderer->RenderText(message, msgX, msgY, msgScale);

    // Draw instructions
    std::string subtitle = "Pressione R para Jogar Novamente ou ESC para Sair";
    float subScale = 0.6f;
    float subWidth = textRenderer->GetTextWidth(subtitle, subScale);
    float subX = (windowWidth - subWidth) / 2.0f;
    float subY = msgY + 60.0f;

    textRenderer->SetTextColor(1.0f, 1.0f, 1.0f); // White
    textRenderer->RenderText(subtitle, subX, subY, subScale);
}
