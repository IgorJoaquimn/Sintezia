#ifndef MAINMENU_H
#define MAINMENU_H

#include <vector>
#include <string>
#include <SDL.h>
#include "../Core/Texture/Texture.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"

class TextRenderer;

class MainMenu {
public:
    MainMenu(SDL_Window* window, SDL_GLContext glContext, TextRenderer* textRenderer);
    ~MainMenu();
    void show();
    int getSelection() const;
private:
    std::vector<std::string> options;
    int selection;
    SDL_Window* mWindow;
    SDL_GLContext mGLContext;
    TextRenderer* mTextRenderer;
    Texture* mLogoTexture;
    SpriteRenderer* mSpriteRenderer;
    void displayOptions() const;
    void displayLogo() const;
    void handleInput(bool& running);
};

#endif // MAINMENU_H
