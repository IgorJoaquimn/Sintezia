// ----------------------------------------------------------------
// Game class following the asteroids game architecture
// ----------------------------------------------------------------

#pragma once
#include <SDL.h>
#include <vector>
#include <memory>
#include "../Math.h"
#include "../Actor/Actor.hpp"
#include "../Core/Renderer/Renderer.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Crafting/Crafting.hpp"

class Game
{
public:
    Game();

    bool Initialize();
    void RunLoop();
    void Shutdown();
    void Quit() { mIsRunning = false; }

    // Actor functions
    void AddActor(std::unique_ptr<Actor> actor);
    void RemoveActor(Actor* actor);
    
    // Get text renderer for measurements
    TextRenderer* GetTextRenderer() { return mTextRenderer.get(); }

    static const int WINDOW_WIDTH = 800;
    static const int WINDOW_HEIGHT = 600;

private:
    void ProcessInput();
    void UpdateGame();
    void GenerateOutput();

    // All the actors in the game
    std::vector<std::unique_ptr<Actor>> mActors;
    std::vector<std::unique_ptr<Actor>> mPendingActors;

    // SDL stuff
    SDL_Window* mWindow;
    SDL_GLContext mGLContext;
    std::unique_ptr<Renderer> mRenderer;
    std::unique_ptr<TextRenderer> mTextRenderer;
    std::unique_ptr<Crafting> mCrafting;

    // Track elapsed time since game start
    Uint32 mTicksCount;

    // Track if we're updating actors right now
    bool mIsRunning;
    bool mUpdatingActors;
};