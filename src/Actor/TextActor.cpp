#include "TextActor.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Game/Game.hpp"

TextActor::TextActor(class Game* game, const std::string& text)
    : Actor(game)
    , mText(text)
{
}

void TextActor::OnDraw(class TextRenderer* textRenderer)
{
    if (textRenderer)
    {
        Vector2 pos = GetPosition();
        // Measure text to get height for baseline positioning
        float textHeight = textRenderer->GetTextHeight(mText, 1.0f);
        // Position text so that top of text is at pos.y (baseline is pos.y + textHeight)
        textRenderer->RenderText(mText, pos.x, pos.y + textHeight, 1.0f);
    }
}