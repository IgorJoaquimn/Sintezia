#include "NPCDialogUI.hpp"
#include "../Game/Game.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include "../Core/Texture/Texture.hpp"
#include <algorithm>

// NPCDialogUI Implementation
NPCDialogUI::NPCDialogUI(Game* game)
    : mGame(game)
    , mState(DialogUIState::Hidden)
    , mPreviousState(DialogUIState::Hidden)
    , mSelectedIndex(0)
    , mOnDialogSelected(nullptr)
    , mOnTradeSelected(nullptr)
    , mOnTalkSelected(nullptr)
    , mOnTradeMenuSelected(nullptr)
    , mOnLeaveSelected(nullptr)
    , mFacesetTexture(nullptr)
{
    // Load UI textures
    mDialogBoxTexture = std::make_shared<Texture>();
    if (!mDialogBoxTexture->Load("assets/third_party/Ninja Adventure - Asset Pack/Ui/Dialog/DialogBox.png")) {
        SDL_Log("Failed to load DialogBox.png");
    }

    mChoiceBoxTexture = std::make_shared<Texture>();
    if (!mChoiceBoxTexture->Load("assets/third_party/Ninja Adventure - Asset Pack/Ui/Dialog/ChoiceBox.png")) {
        SDL_Log("Failed to load ChoiceBox.png");
    }
}

NPCDialogUI::~NPCDialogUI()
{
}

void NPCDialogUI::Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!IsVisible()) return;

    switch (mState)
    {
        case DialogUIState::Greeting:
            DrawGreetingUI(textRenderer, rectRenderer);
            break;
        case DialogUIState::MainMenu:
            DrawMainMenuUI(textRenderer, rectRenderer);
            break;
        case DialogUIState::DialogMenu:
            DrawDialogMenuUI(textRenderer, rectRenderer);
            break;
        case DialogUIState::TradeMenu:
            DrawTradeMenuUI(textRenderer, rectRenderer);
            break;
        case DialogUIState::Message:
            DrawMessageUI(textRenderer, rectRenderer);
            break;
        default:
            break;
    }
}

void NPCDialogUI::Update(float deltaTime)
{
    // Future: Add animations, transitions, etc.
}

void NPCDialogUI::ShowGreeting(const std::string& greeting)
{
    mState = DialogUIState::Greeting;
    mCurrentText = greeting;
    mSelectedIndex = 0;
}

void NPCDialogUI::ShowMainMenu()
{
    mState = DialogUIState::MainMenu;
    mSelectedIndex = 0;
    mCurrentOptions.clear();
    mCurrentOptions = {"Talk", "Trade", "Leave"};
}

void NPCDialogUI::ShowDialogMenu(const std::vector<std::string>& options)
{
    mPreviousState = mState;  // Save current state
    mState = DialogUIState::DialogMenu;
    mCurrentOptions = options;
    mSelectedIndex = 0;
}

void NPCDialogUI::ShowTradeMenu(const std::vector<std::string>& tradeDescriptions)
{
    mPreviousState = mState;  // Save current state
    mState = DialogUIState::TradeMenu;
    mCurrentOptions = tradeDescriptions;
    mSelectedIndex = 0;
}

void NPCDialogUI::ShowMessage(const std::string& message)
{
    mPreviousState = mState;  // Save current state before showing message
    mPreviousOptions = mCurrentOptions;  // Save current options to restore later
    mState = DialogUIState::Message;
    mCurrentText = message;
    mSelectedIndex = 0;
}

void NPCDialogUI::Hide()
{
    mState = DialogUIState::Hidden;
    mSelectedIndex = 0;
    mCurrentText.clear();
    mCurrentOptions.clear();
}

void NPCDialogUI::NavigateUp()
{
    if (mCurrentOptions.empty()) return;
    mSelectedIndex--;
    if (mSelectedIndex < 0)
        mSelectedIndex = mCurrentOptions.size() - 1;
}

void NPCDialogUI::NavigateDown()
{
    if (mCurrentOptions.empty()) return;
    mSelectedIndex++;
    if (mSelectedIndex >= static_cast<int>(mCurrentOptions.size()))
        mSelectedIndex = 0;
}

void NPCDialogUI::SelectCurrent()
{
    switch (mState)
    {
        case DialogUIState::Greeting:
            ShowMainMenu();
            break;

        case DialogUIState::MainMenu:
            if (mSelectedIndex == 0 && mOnTalkSelected)
                mOnTalkSelected();
            else if (mSelectedIndex == 1 && mOnTradeMenuSelected)
                mOnTradeMenuSelected();
            else if (mSelectedIndex == 2 && mOnLeaveSelected)
                mOnLeaveSelected();
            break;

        case DialogUIState::DialogMenu:
            if (mOnDialogSelected && mSelectedIndex >= 0 && mSelectedIndex < static_cast<int>(mCurrentOptions.size()))
                mOnDialogSelected(mSelectedIndex);
            break;

        case DialogUIState::TradeMenu:
            if (mOnTradeSelected && mSelectedIndex >= 0 && mSelectedIndex < static_cast<int>(mCurrentOptions.size()))
                mOnTradeSelected(mSelectedIndex);
            break;

        case DialogUIState::Message:
            // Return to the previous menu (DialogMenu or TradeMenu)
            if (mPreviousState == DialogUIState::DialogMenu)
            {
                // Restore dialog menu with previous options
                mState = DialogUIState::DialogMenu;
                mCurrentOptions = mPreviousOptions;
                mSelectedIndex = 0;
            }
            else if (mPreviousState == DialogUIState::TradeMenu)
            {
                // Restore trade menu with previous options
                mState = DialogUIState::TradeMenu;
                mCurrentOptions = mPreviousOptions;
                mSelectedIndex = 0;
            }
            else
            {
                // Fallback to main menu if we don't know where we came from
                ShowMainMenu();
            }
            break;

        default:
            break;
    }
}

std::vector<std::string> NPCDialogUI::WrapText(const std::string& text, float maxWidth, float scale, TextRenderer* textRenderer)
{
    std::vector<std::string> lines;
    if (!textRenderer || text.empty()) return lines;

    std::string currentLine;
    std::string word;

    for (size_t i = 0; i <= text.length(); i++)
    {
        char c = (i < text.length()) ? text[i] : ' ';

        if (c == ' ' || c == '\n' || i == text.length())
        {
            if (!word.empty())
            {
                std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
                float lineWidth = textRenderer->GetTextWidth(testLine, scale);

                if (lineWidth > maxWidth && !currentLine.empty())
                {
                    // Current word doesn't fit, push current line and start new one
                    lines.push_back(currentLine);
                    currentLine = word;
                }
                else
                {
                    currentLine = testLine;
                }
                word.clear();
            }

            if (c == '\n' && i < text.length())
            {
                lines.push_back(currentLine);
                currentLine.clear();
            }
        }
        else
        {
            word += c;
        }
    }

    if (!currentLine.empty())
    {
        lines.push_back(currentLine);
    }

    return lines;
}

void NPCDialogUI::RenderWrappedText(const std::string& text, float x, float y, float maxWidth, float scale, float lineSpacing, TextRenderer* textRenderer)
{
    if (!textRenderer) return;

    std::vector<std::string> lines = WrapText(text, maxWidth, scale, textRenderer);

    for (size_t i = 0; i < lines.size(); i++)
    {
        textRenderer->RenderText(lines[i], x, y + static_cast<float>(i) * lineSpacing, scale);
    }
}

void NPCDialogUI::DrawBox(RectRenderer* rectRenderer, float x, float y, float width, float height, const Vector3& color, float alpha)
{
    if (rectRenderer)
        rectRenderer->RenderRect(x, y, width, height, color, alpha);
}

void NPCDialogUI::DrawGreetingUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Settings
    float textScale = 0.5f;
    float lineSpacing = 20.0f;
    float uiScale = 2.5f;

    // Margins (Source Pixels)
    float marginTop = 14.0f;
    float marginBottom = 6.0f;
    float marginLeftFace = 50.0f; // Width of face area
    float marginRight = 20.0f;    // Right margin to prevent text leaking

    // Calculate Dialog Box Dimensions based on texture
    float boxWidth = 800.0f;
    float boxHeight = 200.0f;

    if (mDialogBoxTexture) {
        boxWidth = mDialogBoxTexture->GetWidth() * uiScale;
        boxHeight = mDialogBoxTexture->GetHeight() * uiScale;
    }

    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    // Draw Dialog Box (Bottom Layer)
    if (mDialogBoxTexture && mGame->GetSpriteRenderer()) {
        Vector2 pos(boxX, boxY);
        Vector2 size(boxWidth, boxHeight);
        mGame->GetSpriteRenderer()->DrawSprite(mDialogBoxTexture.get(), pos, size, 0.0f, Vector3(1.0f, 1.0f, 1.0f));
    } else {
        DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);
    }

    // Initialize text layout variables
    float textX = boxX + (marginRight * uiScale); 
    // Add baseline offset (approx 20px) because RenderText uses baseline Y
    float textY = boxY + (marginTop * uiScale) + 20.0f;
    float textWidth = boxWidth - (marginRight * uiScale * 2.0f);

    // 2. Handle Faceset (Layer 2)
    if (mFacesetTexture && mGame->GetSpriteRenderer())
    {
        // User measurements (Texture Space)
        float faceMarginLeft = 6.0f; // (50 - 38) / 2 = 6px
        
        // Calculate source rect for the face
        int w = mFacesetTexture->GetWidth();
        int h = mFacesetTexture->GetHeight();
        int faceSize = std::min(w, h); 
        
        float normW = static_cast<float>(faceSize) / static_cast<float>(w);
        float normH = static_cast<float>(faceSize) / static_cast<float>(h);
        Vector2 srcPos(0.0f, 0.0f);
        Vector2 srcSize(normW, normH);

        // Define face display size (38px * scale)
        float faceDisplaySize = faceSize * uiScale; 
        
        // Position X: 6px padding from left (scaled)
        float faceX = boxX + (faceMarginLeft * uiScale);
        
        // Position Y: Respecting the 14px top margin (User specified 14px top, 6px bottom, 38px face height -> 14+38+6 = 58 total height)
        float faceY = boxY + (marginTop * uiScale);

        // Ensure textY is aligned with faceY but shifted down for baseline
        textY = faceY + 20.0f;

        // Draw the face
        mGame->GetSpriteRenderer()->DrawSprite(mFacesetTexture.get(), Vector2(faceX, faceY), Vector2(faceDisplaySize, faceDisplaySize), srcPos, srcSize);
        
        // Adjust text position to start after the 50px left edge
        textX = boxX + (marginLeftFace * uiScale);
        textWidth = boxWidth - (marginLeftFace * uiScale) - (marginRight * uiScale);
    }

    // Draw greeting text
    textRenderer->SetTextColor(0.08f, 0.11f, 0.11f); // #141b1b
    RenderWrappedText(mCurrentText, textX, textY, textWidth, textScale, lineSpacing, textRenderer);
}

void NPCDialogUI::DrawMainMenuUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Settings
    float textScale = 0.45f;
    float uiScale = 2.5f;

    // Margins (Source Pixels)
    float marginTop = 14.0f;
    float marginBottom = 6.0f;
    float marginLeft = 20.0f;
    float marginRight = 20.0f;

    // Calculate Dialog Box Dimensions based on texture
    float boxWidth = 800.0f;
    float boxHeight = 200.0f;

    if (mDialogBoxTexture) {
        boxWidth = mDialogBoxTexture->GetWidth() * uiScale;
        boxHeight = mDialogBoxTexture->GetHeight() * uiScale;
    }

    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    if (mDialogBoxTexture && mGame->GetSpriteRenderer()) {
        Vector2 pos(boxX, boxY);
        Vector2 size(boxWidth, boxHeight);
        mGame->GetSpriteRenderer()->DrawSprite(mDialogBoxTexture.get(), pos, size, 0.0f, Vector3(1.0f, 1.0f, 1.0f));
    } else {
        DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);
    }

    // Draw title
    float textX = boxX + (marginLeft * uiScale);
    float textY = boxY + (marginTop * uiScale) + 20.0f; // Add baseline offset
    textRenderer->SetTextColor(0.08f, 0.11f, 0.11f); // #141b1b
    textRenderer->RenderText("O que você gostaria de fazer?", textX, textY, textScale);

    // Draw options horizontally
    float optionY = textY + 35.0f;
    float currentX = textX;
    float buttonSpacing = 10.0f; // Spacing between buttons

    for (size_t i = 0; i < mCurrentOptions.size(); i++)
    {
        bool isSelected = (i == static_cast<size_t>(mSelectedIndex));
        std::string optionText = mCurrentOptions[i];
        
        // Calculate text size
        Vector2 textSize = textRenderer->MeasureText(optionText, textScale);
        
        // Determine button size (add padding)
        float buttonPaddingX = 15.0f;
        float buttonPaddingY = 10.0f;
        float buttonWidth = textSize.x + (buttonPaddingX * 2);
        float buttonHeight = textSize.y + (buttonPaddingY * 2);

        // Use ChoiceBox texture if available
        if (mChoiceBoxTexture && mGame->GetSpriteRenderer()) {
            // If using texture, we might want to scale it to fit the text or use a fixed size
            // For now, let's stretch it to fit the text + padding
            // Or use a fixed scale if the texture is designed for it.
            // Assuming 9-slice or simple stretch. Let's try simple stretch first.
            
            // Center the button vertically relative to the text line
            float buttonY = optionY - buttonPaddingY; 
            
            // Draw button background
            Vector2 btnPos(currentX, buttonY);
            Vector2 btnSize(buttonWidth, buttonHeight);
            
            // Highlight effect (maybe tint or scale)
            Vector3 color = isSelected ? Vector3(1.0f, 1.0f, 1.0f) : Vector3(0.9f, 0.9f, 0.9f);
            if (isSelected) {
                // Pulse effect or just brighter
                 mGame->GetSpriteRenderer()->DrawSprite(mChoiceBoxTexture.get(), btnPos, btnSize, 0.0f, Vector3(1.2f, 1.2f, 1.2f));
            } else {
                 mGame->GetSpriteRenderer()->DrawSprite(mChoiceBoxTexture.get(), btnPos, btnSize, 0.0f, Vector3(0.8f, 0.8f, 0.8f));
            }
        }

        // Draw text centered in the button
        if (isSelected)
            textRenderer->SetTextColor(1.0f, 1.0f, 0.0f);
        else
            textRenderer->SetTextColor(0.08f, 0.11f, 0.11f); // #141b1b

        // Add offset to center text vertically in the button
        textRenderer->RenderText(optionText, currentX + buttonPaddingX, optionY + 8.0f, textScale);

        // Advance X position
        currentX += buttonWidth + buttonSpacing;
    }
}

void NPCDialogUI::DrawDialogMenuUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Settings
    float textScale = 0.45f;
    float lineHeight = 24.0f;
    float uiScale = 2.5f;

    // Margins (Source Pixels)
    float marginTop = 14.0f;
    float marginBottom = 6.0f;
    float marginLeft = 20.0f;
    float marginRight = 20.0f;

    // Calculate Dialog Box Dimensions based on texture
    float boxWidth = 800.0f;
    float boxHeight = 200.0f;

    if (mDialogBoxTexture) {
        boxWidth = mDialogBoxTexture->GetWidth() * uiScale;
        boxHeight = mDialogBoxTexture->GetHeight() * uiScale;
    }

    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    if (mDialogBoxTexture && mGame->GetSpriteRenderer()) {
        Vector2 pos(boxX, boxY);
        Vector2 size(boxWidth, boxHeight);
        mGame->GetSpriteRenderer()->DrawSprite(mDialogBoxTexture.get(), pos, size, 0.0f, Vector3(1.0f, 1.0f, 1.0f));
    } else {
        DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);
    }

    // Draw title
    float textX = boxX + (marginLeft * uiScale);
    float textY = boxY + (marginTop * uiScale) + 20.0f; // Add baseline offset
    textRenderer->SetTextColor(0.08f, 0.11f, 0.11f); // #141b1b
    textRenderer->RenderText("Choose a topic:", textX, textY, textScale);

    // Draw dialog options with wrapping
    float optionY = textY + 28.0f;
    float maxTextWidth = boxWidth - (marginLeft * uiScale) - (marginRight * uiScale);

    for (size_t i = 0; i < mCurrentOptions.size(); i++)
    {
        std::string prefix = (i == static_cast<size_t>(mSelectedIndex)) ? "> " : "  ";
        bool isSelected = (i == static_cast<size_t>(mSelectedIndex));

        if (isSelected)
            textRenderer->SetTextColor(1.0f, 1.0f, 0.0f);
        else
            textRenderer->SetTextColor(0.08f, 0.11f, 0.11f); // #141b1b

        std::string fullText = prefix + mCurrentOptions[i];

        // Check if text fits, if not wrap it
        if (textRenderer->GetTextWidth(fullText, textScale) > maxTextWidth)
        {
            // Truncate with ellipsis for now (proper wrapping would require multi-line per option)
            std::string truncated = prefix;
            for (char c : mCurrentOptions[i])
            {
                std::string test = truncated + c + "...";
                if (textRenderer->GetTextWidth(test, textScale) > maxTextWidth) break;
                truncated += c;
            }
            fullText = truncated + "...";
        }

        textRenderer->RenderText(fullText, textX, optionY + static_cast<float>(i) * lineHeight, textScale);
    }
}

void NPCDialogUI::DrawTradeMenuUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Settings
    float textScale = 0.4f;
    float itemSpacing = 45.0f;
    float lineSpacing = 18.0f;
    float uiScale = 2.5f;

    // Margins (Source Pixels)
    float marginTop = 14.0f;
    float marginBottom = 6.0f;
    float marginLeft = 20.0f;
    float marginRight = 20.0f;

    // Calculate Dialog Box Dimensions based on texture
    float boxWidth = 800.0f;
    float boxHeight = 200.0f;

    if (mDialogBoxTexture) {
        boxWidth = mDialogBoxTexture->GetWidth() * uiScale;
        boxHeight = mDialogBoxTexture->GetHeight() * uiScale;
    }

    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    if (mDialogBoxTexture && mGame->GetSpriteRenderer()) {
        Vector2 pos(boxX, boxY);
        Vector2 size(boxWidth, boxHeight);
        mGame->GetSpriteRenderer()->DrawSprite(mDialogBoxTexture.get(), pos, size, 0.0f, Vector3(1.0f, 1.0f, 1.0f));
    } else {
        DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);
    }

    // Draw title
    float textX = boxX + (marginLeft * uiScale);
    float textY = boxY + (marginTop * uiScale) + 20.0f; // Add baseline offset
    textRenderer->SetTextColor(0.08f, 0.11f, 0.11f); // #141b1b
    textRenderer->RenderText("Available Trades:", textX, textY, textScale);

    // Draw trade offers with wrapping
    float optionY = textY + 28.0f;
    float maxTextWidth = boxWidth - (marginLeft * uiScale) - (marginRight * uiScale);

    for (size_t i = 0; i < mCurrentOptions.size(); i++)
    {
        bool isSelected = (i == static_cast<size_t>(mSelectedIndex));
        std::string prefix = isSelected ? "> " : "  ";

        if (isSelected)
            textRenderer->SetTextColor(1.0f, 1.0f, 0.0f);
        else
            textRenderer->SetTextColor(0.08f, 0.11f, 0.11f); // #141b1b

        // Wrap text for trade description
        std::vector<std::string> lines = WrapText(mCurrentOptions[i], maxTextWidth, textScale, textRenderer);

        float currentY = optionY + static_cast<float>(i) * itemSpacing;
        for (size_t lineIdx = 0; lineIdx < lines.size(); lineIdx++)
        {
            std::string lineText = (lineIdx == 0) ? prefix + lines[lineIdx] : "  " + lines[lineIdx];
            textRenderer->RenderText(lineText, textX, currentY + static_cast<float>(lineIdx) * lineSpacing, textScale);
        }
    }
}

void NPCDialogUI::DrawMessageUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Settings
    float textScale = 0.5f;
    float lineSpacing = 20.0f;
    float uiScale = 2.5f;

    // Margins (Source Pixels)
    float marginTop = 14.0f;
    float marginBottom = 6.0f;
    float marginLeft = 20.0f;
    float marginRight = 20.0f;

    // Calculate Dialog Box Dimensions based on texture
    float boxWidth = 800.0f;
    float boxHeight = 800.0f;

    if (mDialogBoxTexture) {
        boxWidth = mDialogBoxTexture->GetWidth() * uiScale;
        boxHeight = mDialogBoxTexture->GetHeight() * uiScale;
    }

    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    if (mDialogBoxTexture && mGame->GetSpriteRenderer()) {
        Vector2 pos(boxX, boxY);
        Vector2 size(boxWidth, boxHeight);
        mGame->GetSpriteRenderer()->DrawSprite(mDialogBoxTexture.get(), pos, size, 0.0f, Vector3(1.0f, 1.0f, 1.0f));
    } else {
        DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);
    }

    // Draw message text with wrapping
    float textX = boxX + (marginLeft * uiScale);
    float textY = boxY + (marginTop * uiScale) + 20.0f; // Add baseline offset
    float maxTextWidth = boxWidth - (marginLeft * uiScale) - (marginRight * uiScale);

    textRenderer->SetTextColor(0.08f, 0.11f, 0.11f); // #141b1b
    RenderWrappedText(mCurrentText, textX, textY, maxTextWidth, textScale, lineSpacing, textRenderer);
}

void NPCDialogUI::SetFacesetTexture(const std::string& path)
{
    mFacesetTexture = std::make_shared<Texture>();
    if (!mFacesetTexture->Load(path))
    {
        SDL_Log("Failed to load faceset: %s", path.c_str());
        mFacesetTexture.reset();
    }
}

// InteractionIndicator Implementation
InteractionIndicator::InteractionIndicator(Game* game)
    : mGame(game)
    , mIsVisible(false)
    , mWorldPosition(0.0f, 0.0f)
    , mScreenPosition(0.0f, 0.0f)
    , mAnimTime(0.0f)
    , mAnimFrame(0)
    , mMaxFrames(4)  // DialogInfo.png has 4 animation frames (empty, ., .., ...)
    , mAnimSpeed(4.0f)  // 4 fps for smooth bubble animation
{
    // Load DialogInfo texture
    mDialogInfoTexture = std::make_shared<Texture>();
    if (!mDialogInfoTexture->Load("assets/third_party/Ninja Adventure - Asset Pack/Ui/Dialog/DialogInfo.png")) {
        SDL_Log("Failed to load DialogInfo.png");
        mDialogInfoTexture.reset();
    }
}

InteractionIndicator::~InteractionIndicator()
{
}

void InteractionIndicator::Show(const Vector2& worldPosition)
{
    mIsVisible = true;
    mWorldPosition = worldPosition;
    UpdateScreenPosition();
}

void InteractionIndicator::Hide()
{
    mIsVisible = false;
}

void InteractionIndicator::Update(float deltaTime)
{
    if (!mIsVisible) return;

    // Update animation
    mAnimTime += deltaTime;
    float frameTime = 1.0f / mAnimSpeed;

    if (mAnimTime >= frameTime)
    {
        mAnimTime -= frameTime;
        mAnimFrame = (mAnimFrame + 1) % mMaxFrames;
    }

    // Update screen position (in case world/camera changes)
    UpdateScreenPosition();
}

void InteractionIndicator::Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!mIsVisible || !mDialogInfoTexture || !mGame->GetSpriteRenderer()) return;

    // DialogInfo.png is typically a horizontal sprite sheet with multiple frames
    // Each frame is usually 16x16 pixels
    int frameWidth = 20;
    int frameHeight = 16;

    // Calculate source rectangle for current frame
    // Frames are arranged horizontally in the sprite sheet
    float texWidth = static_cast<float>(mDialogInfoTexture->GetWidth());
    float texHeight = static_cast<float>(mDialogInfoTexture->GetHeight());

    // Normalize coordinates for source rect (0.0 to 1.0)
    float srcX = (static_cast<float>(mAnimFrame * frameWidth)) / texWidth;
    float srcY = 0.0f;
    float srcW = static_cast<float>(frameWidth) / texWidth;
    float srcH = static_cast<float>(frameHeight) / texHeight;

    Vector2 srcPos(srcX, srcY);
    Vector2 srcSize(srcW, srcH);

    // Position above the NPC's head
    float spriteRenderSize = 80.0f;
    float topOfSprite = mScreenPosition.y - spriteRenderSize * 0.5f;

    // Scale up the bubble sprite (2x scale looks good)
    float bubbleScale = 2.5f;
    float bubbleWidth = frameWidth * bubbleScale;
    float bubbleHeight = frameHeight * bubbleScale;

    // Center the bubble above the NPC
    float bubbleX = mScreenPosition.x - bubbleWidth * 0.5f;
    float bubbleY = topOfSprite - 20.0f - bubbleHeight; // 20px gap above sprite

    Vector2 position(bubbleX, bubbleY);
    Vector2 size(bubbleWidth, bubbleHeight);

    // Draw the animated sprite
    mGame->GetSpriteRenderer()->DrawSprite(
        mDialogInfoTexture.get(),
        position,
        size,
        srcPos,
        srcSize,
        0.0f,  // rotation
        Vector3(1.0f, 1.0f, 1.0f)  // white color (no tint)
    );
}

void InteractionIndicator::UpdateScreenPosition()
{
    // For now, assume world position = screen position
    // In a proper implementation, you'd convert world coordinates to screen coordinates
    // using the camera transform
    mScreenPosition = mWorldPosition;
}

