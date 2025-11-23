#pragma once
#include "../MathUtils.h"
#include <string>
#include <vector>
#include <functional>

// Forward declarations
class Game;
class TextRenderer;
class RectRenderer;

// Represents a button in the UI
struct UIButtonData
{
    std::string text;
    Vector2 position;
    Vector2 size;
    std::function<void()> onClick;
    bool isHighlighted;

    UIButtonData(const std::string& txt, const Vector2& pos, const Vector2& sz, std::function<void()> callback)
        : text(txt), position(pos), size(sz), onClick(callback), isHighlighted(false) {}
};

// Dialog UI State
enum class DialogUIState
{
    Hidden,
    Greeting,
    MainMenu,
    DialogMenu,
    TradeMenu,
    Message
};

// Main NPC Dialog UI Manager
class NPCDialogUI
{
public:
    NPCDialogUI(Game* game);
    ~NPCDialogUI();

    void Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer);
    void Update(float deltaTime);

    // State management
    void ShowGreeting(const std::string& greeting);
    void ShowMainMenu();
    void ShowDialogMenu(const std::vector<std::string>& options);
    void ShowTradeMenu(const std::vector<std::string>& tradeDescriptions);
    void ShowMessage(const std::string& message);
    void Hide();

    bool IsVisible() const { return mState != DialogUIState::Hidden; }
    DialogUIState GetState() const { return mState; }

    // Navigation
    void NavigateUp();
    void NavigateDown();
    void SelectCurrent();
    int GetSelectedIndex() const { return mSelectedIndex; }

    // Set callbacks
    void SetOnDialogSelected(std::function<void(int)> callback) { mOnDialogSelected = callback; }
    void SetOnTradeSelected(std::function<void(int)> callback) { mOnTradeSelected = callback; }
    void SetOnTalkSelected(std::function<void()> callback) { mOnTalkSelected = callback; }
    void SetOnTradeMenuSelected(std::function<void()> callback) { mOnTradeMenuSelected = callback; }
    void SetOnLeaveSelected(std::function<void()> callback) { mOnLeaveSelected = callback; }

private:
    Game* mGame;
    DialogUIState mState;
    DialogUIState mPreviousState;  // Track which menu we came from
    int mSelectedIndex;

    // Current display data
    std::string mCurrentText;
    std::vector<std::string> mCurrentOptions;
    std::vector<std::string> mPreviousOptions;  // Store previous options to restore menu

    // Callbacks
    std::function<void(int)> mOnDialogSelected;
    std::function<void(int)> mOnTradeSelected;
    std::function<void()> mOnTalkSelected;
    std::function<void()> mOnTradeMenuSelected;
    std::function<void()> mOnLeaveSelected;

    // Helper rendering methods
    void DrawBox(RectRenderer* rectRenderer, float x, float y, float width, float height, const Vector3& color, float alpha);
    void DrawGreetingUI(TextRenderer* textRenderer, RectRenderer* rectRenderer);
    void DrawMainMenuUI(TextRenderer* textRenderer, RectRenderer* rectRenderer);
    void DrawDialogMenuUI(TextRenderer* textRenderer, RectRenderer* rectRenderer);
    void DrawTradeMenuUI(TextRenderer* textRenderer, RectRenderer* rectRenderer);
    void DrawMessageUI(TextRenderer* textRenderer, RectRenderer* rectRenderer);

    // Text wrapping utilities
    std::vector<std::string> WrapText(const std::string& text, float maxWidth, float scale, TextRenderer* textRenderer);
    void RenderWrappedText(const std::string& text, float x, float y, float maxWidth, float scale, float lineSpacing, TextRenderer* textRenderer);
};

// Interaction indicator UI - shows "[E] to interact" when player is near NPC
class InteractionIndicator
{
public:
    InteractionIndicator(Game* game);
    ~InteractionIndicator();

    void Show(const Vector2& worldPosition);
    void Hide();
    void Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer);

    bool IsVisible() const { return mIsVisible; }

private:
    Game* mGame;
    bool mIsVisible;
    Vector2 mWorldPosition;
    Vector2 mScreenPosition;

    void UpdateScreenPosition();
};

