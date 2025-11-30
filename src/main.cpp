// ----------------------------------------------------------------
// Main entry point following the asteroids game architecture
// ----------------------------------------------------------------

#include "Game/Game.hpp"
#include "UI/MainMenu.h"
#include "Core/TextRenderer/TextRenderer.hpp"
#include <SDL.h>
#include <GL/glew.h>

int main(int argc, char** argv)
{
    // Inicialização SDL e OpenGL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_Window* window = SDL_CreateWindow("Infinite Craft Clone", 100, 100, Game::WINDOW_WIDTH, Game::WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        SDL_Log("Failed to create OpenGL context: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        SDL_Log("Failed to initialize GLEW");
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    // Inicializa TextRenderer
    TextRenderer textRenderer;
    textRenderer.Initialize(Game::WINDOW_WIDTH, Game::WINDOW_HEIGHT);
    // Exibe o menu
    MainMenu menu(window, glContext, &textRenderer);
    menu.show();
    // Se o usuário escolher Iniciar Jogo
    if (menu.getSelection() == 0) {
        Game game(window, glContext);
        bool success = game.Initialize();
        if (success) {
            game.RunLoop();
        }
        game.Shutdown();
    }
    // Finalização
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}