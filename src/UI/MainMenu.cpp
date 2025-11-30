#include "MainMenu.h"
#include <iostream>
#include <SDL.h>
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/Texture/Texture.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"

MainMenu::MainMenu(SDL_Window* window, SDL_GLContext glContext, TextRenderer* textRenderer)
    : selection(0), mWindow(window), mGLContext(glContext), mTextRenderer(textRenderer), mBackgroundTexture(nullptr), mSpriteRenderer(nullptr)
{
    options = {"Iniciar Jogo", "Sair"};
    
    mBackgroundTexture = new Texture();
    if (!mBackgroundTexture->Load("assets/fundo.png")) {
        // Try fallback
        mBackgroundTexture->Load("../assets/fundo.png");
    }

    mSpriteRenderer = new SpriteRenderer();
    mSpriteRenderer->Initialize((float)textRenderer->GetWindowWidth(), (float)textRenderer->GetWindowHeight());
}

MainMenu::~MainMenu() {
    if (mBackgroundTexture) {
        mBackgroundTexture->Unload();
        delete mBackgroundTexture;
    }
    if (mSpriteRenderer) {
        mSpriteRenderer->Shutdown();
        delete mSpriteRenderer;
    }
}

void MainMenu::show() {
    bool running = true;
    while (running) {
        // Limpa a tela
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Renderiza o fundo
        displayBackground();

        // Renderiza as opções
        displayOptions();
        SDL_GL_SwapWindow(mWindow);
        // Processa entrada
        handleInput(running);
        SDL_Delay(16); // ~60 FPS
    }
}

void MainMenu::displayBackground() const {
    if (!mBackgroundTexture || !mSpriteRenderer) return;
    
    float windowWidth = (float)mTextRenderer->GetWindowWidth();
    float windowHeight = (float)mTextRenderer->GetWindowHeight();
    
    mSpriteRenderer->DrawSprite(mBackgroundTexture, Vector2(0, 0), Vector2(windowWidth, windowHeight));
}

void MainMenu::displayOptions() const {
    if (!mTextRenderer) return;
    float scale = 1.0f;
    float step = 50.0f;
    float totalHeight = (options.size() - 1) * step;
    // Posiciona o centro do menu em 75% da altura da tela (mais para baixo)
    float startY = (mTextRenderer->GetWindowHeight() * 0.75f) - (totalHeight / 2.0f);
    for (size_t i = 0; i < options.size(); ++i) {
        float textWidth = mTextRenderer->GetTextWidth(options[i], scale);
        float x = (mTextRenderer->GetWindowWidth() - textWidth) / 2.0f;
        float y = startY + i * step;
        if ((int)i == selection) {
            mTextRenderer->SetTextColor(1.0f, 1.0f, 0.0f);
        } else {
            mTextRenderer->SetTextColor(1.0f, 1.0f, 1.0f);
        }
        mTextRenderer->RenderText(options[i], x, y, scale);
    }
}

void MainMenu::handleInput(bool& running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            selection = 1; // Sair
            running = false;
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_UP:
                    selection = (selection - 1 + options.size()) % options.size();
                    break;
                case SDLK_DOWN:
                    selection = (selection + 1) % options.size();
                    break;
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    running = false;
                    break;
                default:
                    break;
            }
        }
    }
}

int MainMenu::getSelection() const {
    return selection;
}
