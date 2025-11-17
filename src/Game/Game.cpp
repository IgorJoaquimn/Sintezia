// ----------------------------------------------------------------
// Game implementation following the asteroids game architecture
// ----------------------------------------------------------------

#include "Game.hpp"
#include "../Actor/Actor.hpp"
#include "../Actor/TextActor.hpp"
#include "../Actor/ItemActor.hpp"
#include "../Core/Renderer/Renderer.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RenderUtils.hpp"
#include "../Crafting/Crafting.hpp"
#include <iostream>
#include <algorithm>

Game::Game()
    : mWindow(nullptr)
    , mGLContext(nullptr)
    , mRenderer(nullptr)
    , mTextRenderer(nullptr)
    , mCrafting(nullptr)
    , mTicksCount(0)
    , mIsRunning(true)
    , mUpdatingActors(false)
{
}

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
    if (!mTextRenderer->Initialize())
    {
        SDL_Log("Warning: Failed to initialize text renderer");
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

    // Get items with id 1 and 2
    const Item* item1 = mCrafting->FindItemById(1);
    const Item* item2 = mCrafting->FindItemById(2);
    
    if (item1 && item2)
    {
        float xPos = 50.0f;
        float yPos = 200.0f;
        float spacing = 20.0f; // Space between elements
        
        // Display first item (Water) using ItemActor
        auto actor1 = std::make_unique<ItemActor>(this, *item1);
        actor1->SetPosition(Vector2(xPos, yPos));
        AddActor(std::move(actor1));
        
        // Calculate width of first item to position the "+"
        float item1Width = mTextRenderer->GetTextWidth(item1->emoji + " " + item1->name);
        xPos += item1Width + spacing;
        
        // Display combination symbol
        auto plusActor = std::make_unique<TextActor>(this, "+");
        plusActor->SetPosition(Vector2(xPos, yPos));
        AddActor(std::move(plusActor));
        
        // Move past the "+"
        float plusWidth = mTextRenderer->GetTextWidth("+");
        xPos += plusWidth + spacing;
        
        // Display second item (Fire) using ItemActor
        auto actor2 = std::make_unique<ItemActor>(this, *item2);
        actor2->SetPosition(Vector2(xPos, yPos));
        AddActor(std::move(actor2));
        
        // Calculate width of second item to position the "="
        float item2Width = mTextRenderer->GetTextWidth(item2->emoji + " " + item2->name);
        xPos += item2Width + spacing;
        
        // Combine the items
        auto result = mCrafting->combine_items(*item1, *item2);
        if (result)
        {
            // Display equals
            auto equalsActor = std::make_unique<TextActor>(this, "=");
            equalsActor->SetPosition(Vector2(xPos, yPos));
            AddActor(std::move(equalsActor));
            
            // Move past the "="
            float equalsWidth = mTextRenderer->GetTextWidth("=");
            xPos += equalsWidth + spacing;
            
            // Display result using ItemActor
            auto resultActor = std::make_unique<ItemActor>(this, *result);
            resultActor->SetPosition(Vector2(xPos, yPos));
            AddActor(std::move(resultActor));
        }
    }
    
    // Set different text color for variety
    mTextRenderer->SetTextColor(0.8f, 1.0f, 0.8f); // Light green
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
        }
    }

    const Uint8* state = SDL_GetKeyboardState(nullptr);
    if (state[SDL_SCANCODE_ESCAPE])
    {
        Quit();
    }

    // Process input for all Actors
    mUpdatingActors = true;
    for (auto& actor : mActors)
    {
        actor->ProcessInput(state);
    }
    mUpdatingActors = false;
}

void Game::UpdateGame()
{
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16));

    float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
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
    RenderUtils::ClearScreen(0.1f, 0.1f, 0.1f, 1.0f);
    
    // Render all actors
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
