#include "MainMenu.h"
#include <iostream>
#include <SDL.h>
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/Texture/Texture.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"

MainMenu::MainMenu(SDL_Window* window, SDL_GLContext glContext, TextRenderer* textRenderer)
    : selection(0), mWindow(window), mGLContext(glContext), mTextRenderer(textRenderer), mLogoTexture(nullptr), mSpriteRenderer(nullptr)
{
    options = {"Iniciar Jogo", "Opções", "Sair"};
    mLogoTexture = new Texture();
    mLogoTexture->Load("assets/logo.png");
    mSpriteRenderer = new SpriteRenderer();
    mSpriteRenderer->Initialize((float)textRenderer->GetWindowWidth(), (float)textRenderer->GetWindowHeight());
}

MainMenu::~MainMenu() {
    if (mLogoTexture) {
        mLogoTexture->Unload();
        delete mLogoTexture;
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
        // Renderiza o logo
        displayLogo();
        // Renderiza as opções
        displayOptions();
        SDL_GL_SwapWindow(mWindow);
        // Processa entrada
        handleInput(running);
        SDL_Delay(16); // ~60 FPS
    }
}

void MainMenu::displayLogo() const {
    if (!mLogoTexture || !mSpriteRenderer) return;
    float logoWidth = (float)mLogoTexture->GetWidth();
    float logoHeight = (float)mLogoTexture->GetHeight();
    if (logoWidth <= 1.0f || logoHeight <= 1.0f) {
        std::cerr << "Logo: dimensões inválidas: " << logoWidth << "x" << logoHeight << std::endl;
        return;
    }
    float scale = 0.2f; // Fator de escala para diminuir a logo
    float scaledWidth = logoWidth * scale;
    float scaledHeight = logoHeight * scale;
    float windowWidth = (float)mTextRenderer->GetWindowWidth();
    float x = (windowWidth - scaledWidth) / 2.0f;
    float y = 40.0f; // margem do topo
    mSpriteRenderer->DrawSprite(mLogoTexture, Vector2(x, y), Vector2(scaledWidth, scaledHeight));
}

void MainMenu::displayOptions() const {
    if (!mTextRenderer) return;
    float scale = 1.0f;
    float step = 50.0f;
    float logoHeight = mLogoTexture ? mLogoTexture->GetHeight() * 0.2f : 0.0f; // altura da logo já escalada
    float extraMargin = -200.0f; // margem extra após a logo
    float totalHeight = (options.size() - 1) * step;
    float startY = logoHeight + extraMargin + (mTextRenderer->GetWindowHeight() / 2.0f) - (totalHeight / 2.0f);
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
            selection = 2; // Sair
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
