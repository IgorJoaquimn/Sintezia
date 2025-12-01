#include "ControlsUI.hpp"
#include "../Core/Texture/Texture.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"

ControlsUI::ControlsUI() {
    LoadTextures();
}

ControlsUI::~ControlsUI() {
}

void ControlsUI::LoadTextures() {
    mKeyW = std::make_shared<Texture>();
    mKeyW->Load("assets/third_party/Ninja Adventure - Asset Pack/Ui/Input/Keyboard/KeyW.png");
    
    mKeyA = std::make_shared<Texture>();
    mKeyA->Load("assets/third_party/Ninja Adventure - Asset Pack/Ui/Input/Keyboard/KeyA.png");
    
    mKeyS = std::make_shared<Texture>();
    mKeyS->Load("assets/third_party/Ninja Adventure - Asset Pack/Ui/Input/Keyboard/KeyS.png");
    
    mKeyD = std::make_shared<Texture>();
    mKeyD->Load("assets/third_party/Ninja Adventure - Asset Pack/Ui/Input/Keyboard/KeyD.png");
    
    mKeyI = std::make_shared<Texture>();
    mKeyI->Load("assets/third_party/Ninja Adventure - Asset Pack/Ui/Input/Keyboard/KeyI.png");
    
    mKeyZ = std::make_shared<Texture>();
    mKeyZ->Load("assets/third_party/Ninja Adventure - Asset Pack/Ui/Input/Keyboard/KeyK.png");
}

void ControlsUI::Draw(SpriteRenderer* spriteRenderer, TextRenderer* textRenderer) {
    if (!spriteRenderer || !textRenderer) return;

    float windowHeight = spriteRenderer->GetWindowHeight();
    float windowWidth = spriteRenderer->GetWindowWidth();
    
    // Fallback
    if (windowHeight == 0) windowHeight = 800.0f;
    if (windowWidth == 0) windowWidth = 1200.0f;

    float yPos = windowHeight - 50.0f;
    float startX = 50.0f;
    float keySize = 32.0f;
    float spacing = 5.0f;
    float groupSpacing = 40.0f;
    float textScale = 0.4f;

    textRenderer->SetTextColor(1.0f, 1.0f, 1.0f);

    float currentX = startX;
    
    // Group 1: Movement (WASD)
    spriteRenderer->DrawSprite(mKeyW.get(), Vector2(currentX, yPos), Vector2(keySize, keySize));
    currentX += keySize + spacing;
    spriteRenderer->DrawSprite(mKeyA.get(), Vector2(currentX, yPos), Vector2(keySize, keySize));
    currentX += keySize + spacing;
    spriteRenderer->DrawSprite(mKeyS.get(), Vector2(currentX, yPos), Vector2(keySize, keySize));
    currentX += keySize + spacing;
    spriteRenderer->DrawSprite(mKeyD.get(), Vector2(currentX, yPos), Vector2(keySize, keySize));
    currentX += keySize + spacing;
    
    textRenderer->RenderText("Andar", currentX, yPos + 24.0f, textScale);
    Vector2 textSize = textRenderer->MeasureText("Andar", textScale);
    currentX += textSize.x + groupSpacing;

    // Group 2: Inventory (I)
    spriteRenderer->DrawSprite(mKeyI.get(), Vector2(currentX, yPos), Vector2(keySize, keySize));
    currentX += keySize + spacing;
    
    textRenderer->RenderText("Inventario", currentX, yPos + 24.0f, textScale);
    textSize = textRenderer->MeasureText("Inventario", textScale);
    currentX += textSize.x + groupSpacing;

    // Group 3: Attack/Collect (Z)
    spriteRenderer->DrawSprite(mKeyZ.get(), Vector2(currentX, yPos), Vector2(keySize, keySize));
    currentX += keySize + spacing;
    
    textRenderer->RenderText("Coletar/Atacar", currentX, yPos + 24.0f, textScale);
}
