// ----------------------------------------------------------------
// Game class following the asteroids game architecture
// ----------------------------------------------------------------

#pragma once
#include <SDL.h>
#include <vector>
#include <memory>
#include "../MathUtils.h"
#include "../Actor/Actor.hpp"
#include "../Core/Renderer/Renderer.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include "../Crafting/Crafting.hpp"

// Forward declarations
class TileMap;
class Player;
class NPC;

class Game
{
public:
    Game();
    ~Game();

    bool Initialize();
    void RunLoop();
    void Shutdown();
    void Quit() { mIsRunning = false; }

    // Actor functions
    void AddActor(std::unique_ptr<Actor> actor);
    void RemoveActor(Actor* actor);
    
    // Get text renderer for measurements
    TextRenderer* GetTextRenderer() { return mTextRenderer.get(); }
    const TextRenderer* GetTextRenderer() const { return mTextRenderer.get(); }
    
    // Get rect renderer for backgrounds
    RectRenderer* GetRectRenderer() { return mRectRenderer.get(); }
    
    // Get sprite renderer
    SpriteRenderer* GetSpriteRenderer() { return mSpriteRenderer.get(); }
    
    // Get crafting system
    Crafting* GetCrafting() { return mCrafting.get(); }
    
    // Get player
    Player* GetPlayer() { return mPlayer; }
    
    // Get tilemap
    TileMap* GetTileMap() { return mTileMap.get(); }
    
    // NPC management
    void RegisterNPC(NPC* npc);
    void UnregisterNPC(NPC* npc);
    NPC* GetInteractingNPC() { return mInteractingNPC; }

    // Mouse state
    const Vector2& GetMousePosition() const { return mMousePos; }

    static const int WINDOW_WIDTH = 1200;  // 30 tiles × 40px = 1200px
    static const int WINDOW_HEIGHT = 800;  // 20 tiles × 40px = 800px

private:
    void ProcessInput();
    void UpdateGame();
    void GenerateOutput();
    void CombineItems(class ItemActor* item1, class ItemActor* item2);

    // All the actors in the game
    std::vector<std::unique_ptr<Actor>> mActors;
    std::vector<std::unique_ptr<Actor>> mPendingActors;

    // SDL stuff
    SDL_Window* mWindow;
    SDL_GLContext mGLContext;
    std::unique_ptr<Renderer> mRenderer;
    std::unique_ptr<TextRenderer> mTextRenderer;
    std::unique_ptr<RectRenderer> mRectRenderer;
    std::unique_ptr<SpriteRenderer> mSpriteRenderer;
    std::unique_ptr<Crafting> mCrafting;
    std::unique_ptr<TileMap> mTileMap;

    // Track elapsed time since game start
    Uint32 mTicksCount;

    // Track if we're updating actors right now
    bool mIsRunning;
    bool mUpdatingActors;
    
    // Game objects
    Player* mPlayer;
    
    // NPC tracking
    std::vector<NPC*> mNPCs;
    NPC* mInteractingNPC;

    // Mouse state
    Vector2 mMousePos;
};