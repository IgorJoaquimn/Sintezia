// ----------------------------------------------------------------
// Game implementation following the asteroids game architecture
// ----------------------------------------------------------------

#include "Game.hpp"
#include "../Actor/Actor.hpp"
#include "../Actor/TextActor.hpp"
#include "../Actor/ItemActor.hpp"
#include "../Actor/Player.hpp"
#include "../Actor/NPC/DialogNPC.hpp"
#include "../Actor/NPC/TestShopkeeperNPC.hpp"
#include "../Actor/NPC/TestPassivePatrolNPC.hpp"
#include "../Actor/NPC/TestAggressivePatrolNPC.hpp"
#include "../Map/TileMap.hpp"
#include "../Core/Renderer/Renderer.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include "../Core/RenderUtils.hpp"
#include "../Crafting/Crafting.hpp"
#include <algorithm>

Game::Game()
    : mWindow(nullptr)
    , mGLContext(nullptr)
    , mRenderer(nullptr)
    , mTextRenderer(nullptr)
    , mRectRenderer(nullptr)
    , mSpriteRenderer(nullptr)
    , mCrafting(nullptr)
    , mTileMap(nullptr)
    , mTicksCount(0)
    , mIsRunning(true)
    , mUpdatingActors(false)
    , mPlayer(nullptr)
    , mInteractingNPC(nullptr)
    , mMousePos(Vector2::Zero)
{
}

Game::~Game() = default;

bool Game::Initialize()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    mWindow = SDL_CreateWindow("Infinite Craft Clone", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    // Create OpenGL context
    mGLContext = SDL_GL_CreateContext(mWindow);
    if (!mGLContext)
    {
        SDL_Log("Failed to create OpenGL context: %s", SDL_GetError());
        return false;
    }

    mRenderer = std::make_unique<Renderer>();
    if (!mRenderer->Initialize(WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        SDL_Log("Failed to initialize renderer");
        return false;
    }

    // Initialize text renderer
    mTextRenderer = std::make_unique<TextRenderer>();
    if (!mTextRenderer->Initialize(WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        SDL_Log("Warning: Failed to initialize text renderer");
    }
    
    // Initialize rect renderer
    mRectRenderer = std::make_unique<RectRenderer>();
    if (!mRectRenderer->Initialize(WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        SDL_Log("Warning: Failed to initialize rect renderer");
    }
    
    // Initialize sprite renderer
    mSpriteRenderer = std::make_unique<SpriteRenderer>();
    if (!mSpriteRenderer->Initialize(WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        SDL_Log("Warning: Failed to initialize sprite renderer");
    }

    // Initialize crafting system
    mCrafting = std::make_unique<Crafting>();
    
    // Load items and recipes from JSON
    if (!mCrafting->LoadItemsFromJson("assets/items.json"))
    {
        SDL_Log("Warning: Failed to load items");
    }
    
    if (!mCrafting->LoadRecipesFromJson("assets/recipes.json"))
    {
        SDL_Log("Warning: Failed to load recipes");
    }

    // Create tile map
    // Window is 1200×800, map is 30×20 tiles: perfect fit at 40px per tile
    mTileMap = std::make_unique<TileMap>(30, 20, 40);
    
    // Load your custom Tiled map
    if (!mTileMap->LoadFromJSON("assets/maps/mapa_de_teste.json"))
    {
        SDL_Log("Warning: Failed to load custom map, using procedural generation");
    }
    
    // Create player
    auto player = std::make_unique<Player>(this);
    mPlayer = player.get(); // Safe: player ownership transferred to mActors, pointer valid for game lifetime
    AddActor(std::move(player));

    // Create test shopkeeper NPC (dialog NPC with trading)
    auto testShopkeeperNPC = std::make_unique<TestShopkeeperNPC>(this);
    RegisterNPC(testShopkeeperNPC.get());
    AddActor(std::move(testShopkeeperNPC));

    // Create test passive patrol NPC (patrols in a loop)
    auto testPassivePatrolNPC = std::make_unique<TestPassivePatrolNPC>(this);
    AddActor(std::move(testPassivePatrolNPC));

    // Create test aggressive patrol NPC (patrols and chases player)
    auto testAggressivePatrolNPC = std::make_unique<TestAggressivePatrolNPC>(this);
    AddActor(std::move(testAggressivePatrolNPC));

    // Set different text color for variety
    mTextRenderer->SetTextColor(1.0f, 1.0f, 1.0f); // White
    mTicksCount = SDL_GetTicks();

    return true;
}

void Game::RunLoop()
{
    while (mIsRunning)
    {
        ProcessInput();
        UpdateGame();
        GenerateOutput();
    }
}

void Game::ProcessInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                Quit();
                break;
            default:
                // Ignore other events
                break;
        }
    }
    
    // Process keyboard state for player movement
    const Uint8* keyState = SDL_GetKeyboardState(nullptr);

    // Check for NPC interaction
    if (mInteractingNPC && mInteractingNPC->IsInteracting())
    {
        // If interacting with an NPC, pass input to the NPC
        mInteractingNPC->HandleInteractionInput(keyState);
    }
    else
    {
        // Static variable to track E key press
        static bool eKeyPressed = false;

        // Check for nearby NPCs and show interaction indicator
        DialogNPC* nearbyNPC = nullptr;
        if (mPlayer)
        {
            for (DialogNPC* npc : mNPCs)
            {
                if (npc->CanInteract(mPlayer->GetPosition()))
                {
                    nearbyNPC = npc;
                    npc->ShowInteractionIndicator(mPlayer->GetPosition());
                    break;
                }
                else
                {
                    npc->HideInteractionIndicator();
                }
            }
        }

        // Check for E key press to interact with nearby NPCs
        if (keyState[SDL_SCANCODE_E] && !eKeyPressed)
        {
            eKeyPressed = true;

            if (nearbyNPC)
            {
                nearbyNPC->StartInteraction();
                mInteractingNPC = nearbyNPC;
            }
        }
        else if (!keyState[SDL_SCANCODE_E])
        {
            eKeyPressed = false;
        }

        // Process player input only when not interacting
        if (mPlayer)
        {
            mPlayer->ProcessInput(keyState);
        }
    }
}

void Game::UpdateGame()
{
    // Wait until 16ms have passed (frame limiting)
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16))
    {
        // Busy wait
    }

    float deltaTime = static_cast<float>(SDL_GetTicks() - mTicksCount) / 1000.0f;
    if (deltaTime > 0.05f)
    {
        deltaTime = 0.05f;
    }

    mTicksCount = SDL_GetTicks();

    // Update all actors
    mUpdatingActors = true;

    for (auto& actor : mActors)
    {
        actor->Update(deltaTime);
    }

    mUpdatingActors = false;

    // Move pending actors to mActors
    for (auto& pending : mPendingActors)
    {
        mActors.emplace_back(std::move(pending));
    }
    mPendingActors.clear();

    // Remove dead actors
    mActors.erase(
        std::remove_if(mActors.begin(), mActors.end(),
            [](const std::unique_ptr<Actor>& actor) {
                return actor->GetState() == ActorState::Destroy;
            }),
        mActors.end()
    );
}

void Game::GenerateOutput()
{
    mRenderer->BeginFrame();
    
    // Use common render utility for screen clearing
    RenderUtils::ClearScreen(0.2f, 0.5f, 0.3f, 1.0f); // Green-ish background
    
    // Draw tilemap first
    if (mTileMap)
    {
        mTileMap->Draw(mSpriteRenderer.get());
    }
    
    // Render all actors on top
    for (auto& actor : mActors)
    {
        if (actor->GetState() == ActorState::Active)
        {
            actor->OnDraw(mTextRenderer.get());
        }
    }
    
    mRenderer->EndFrame();
    
    SDL_GL_SwapWindow(mWindow);
}

void Game::AddActor(std::unique_ptr<Actor> actor)
{
    if (mUpdatingActors)
    {
        mPendingActors.emplace_back(std::move(actor));
    }
    else
    {
        mActors.emplace_back(std::move(actor));
    }
}

void Game::RemoveActor(Actor* actor)
{
    auto it = std::find_if(mActors.begin(), mActors.end(),
        [actor](const std::unique_ptr<Actor>& a) {
            return a.get() == actor;
        });

    if (it != mActors.end())
    {
        mActors.erase(it);
    }

    auto pendingIt = std::find_if(mPendingActors.begin(), mPendingActors.end(),
        [actor](const std::unique_ptr<Actor>& a) {
            return a.get() == actor;
        });
    
    if (pendingIt != mPendingActors.end())
    {
        mPendingActors.erase(pendingIt);
    }
}

void Game::Shutdown()
{
    // Clear actors (smart pointers will automatically clean up)
    mIsRunning = false;
    mActors.clear();
    mPendingActors.clear();

    if (mTextRenderer)
    {
        mTextRenderer.reset();
    }

    if (mRectRenderer)
    {
        mRectRenderer->Shutdown();
        mRectRenderer.reset();
    }

    if (mCrafting)
    {
        mCrafting.reset();
    }

    if (mRenderer)
    {
        mRenderer->Shutdown();
        mRenderer.reset();
    }

    // Cleanup OpenGL context
    if (mGLContext)
    {
        SDL_GL_DeleteContext(mGLContext);
        mGLContext = nullptr;
    }

    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}

void Game::RegisterNPC(DialogNPC* npc)
{
    mNPCs.push_back(npc);
}

void Game::UnregisterNPC(DialogNPC* npc)
{
    auto it = std::find(mNPCs.begin(), mNPCs.end(), npc);
    if (it != mNPCs.end())
    {
        if (mInteractingNPC == npc)
        {
            mInteractingNPC = nullptr;
        }
        mNPCs.erase(it);
    }
}

void Game::CombineItems(ItemActor* item1, ItemActor* item2)
{
    if (!item1 || !item2 || !mCrafting)
        return;
    
    // Try to combine the items
    auto result = mCrafting->combine_items(item1->GetItem(), item2->GetItem());
    
    if (result)
    {
        // Calculate position for the new item (midpoint between the two)
        Vector2 pos1 = item1->GetPosition();
        Vector2 pos2 = item2->GetPosition();
        Vector2 newPos = Vector2((pos1.x + pos2.x) / 2.0f, (pos1.y + pos2.y) / 2.0f);
        
        // Create the result item actor
        auto resultActor = std::make_unique<ItemActor>(this, *result);
        resultActor->SetPosition(newPos);
        AddActor(std::move(resultActor));
        
        // Mark the original items for destruction
        item1->SetState(ActorState::Destroy);
        item2->SetState(ActorState::Destroy);
        
        SDL_Log("Combined %s + %s = %s", 
                item1->GetItem().name.c_str(), 
                item2->GetItem().name.c_str(), 
                result->name.c_str());
    }
}
