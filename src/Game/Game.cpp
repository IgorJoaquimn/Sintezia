// ----------------------------------------------------------------
// Game implementation following the asteroids game architecture
// ----------------------------------------------------------------

#include "Game.hpp"
#include "../Actor/Actor.hpp"
#include "../Actor/TextActor.hpp"
#include "../Actor/ItemActor.hpp"
#include "../Core/Renderer/Renderer.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include "../Core/RenderUtils.hpp"
#include "../Crafting/Crafting.hpp"
#include <iostream>
#include <algorithm>

Game::Game()
    : mWindow(nullptr)
    , mGLContext(nullptr)
    , mRenderer(nullptr)
    , mTextRenderer(nullptr)
    , mRectRenderer(nullptr)
    , mCrafting(nullptr)
    , mTicksCount(0)
    , mIsRunning(true)
    , mUpdatingActors(false)
    , mMousePos(Vector2::Zero)
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
        
        // Position second item next to it
        xPos += 250.0f; // Simple spacing
        
        // Display second item (Fire) using ItemActor
        auto actor2 = std::make_unique<ItemActor>(this, *item2);
        actor2->SetPosition(Vector2(xPos, yPos));
        AddActor(std::move(actor2));
    }
    
    // Set different text color for variety
    mTextRenderer->SetTextColor(1.0f, 1.0f, 1.0f); // Light yellow
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
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mMousePos.x = static_cast<float>(event.button.x);
                    mMousePos.y = static_cast<float>(event.button.y);
                    
                    // Iterate through actors in reverse order (top to bottom)
                    for (auto it = mActors.rbegin(); it != mActors.rend(); ++it)
                    {
                        ItemActor* itemActor = dynamic_cast<ItemActor*>(it->get());
                        if (itemActor && itemActor->IsDraggable())
                        {
                            itemActor->OnMouseDown(mMousePos);
                            if (itemActor->IsDragging())
                            {
                                break; // Only drag the topmost item
                            }
                        }
                    }
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mMousePos.x = static_cast<float>(event.button.x);
                    mMousePos.y = static_cast<float>(event.button.y);
                    
                    // Track which item was just released for collision checking
                    ItemActor* releasedItem = nullptr;
                    
                    for (auto& actor : mActors)
                    {
                        ItemActor* itemActor = dynamic_cast<ItemActor*>(actor.get());
                        if (itemActor && itemActor->IsDragging())
                        {
                            releasedItem = itemActor;
                        }
                        if (itemActor)
                        {
                            itemActor->OnMouseUp(mMousePos);
                        }
                    }
                    
                    // Check for collision immediately after release
                    if (releasedItem)
                    {
                        for (auto& actor : mActors)
                        {
                            ItemActor* otherItem = dynamic_cast<ItemActor*>(actor.get());
                            if (otherItem && otherItem != releasedItem && 
                                otherItem->GetState() == ActorState::Active &&
                                releasedItem->Intersects(otherItem))
                            {
                                CombineItems(releasedItem, otherItem);
                                break; // Only combine with one item
                            }
                        }
                    }
                }
                break;
            case SDL_MOUSEMOTION:
                mMousePos.x = static_cast<float>(event.motion.x);
                mMousePos.y = static_cast<float>(event.motion.y);
                
                for (auto& actor : mActors)
                {
                    ItemActor* itemActor = dynamic_cast<ItemActor*>(actor.get());
                    if (itemActor && itemActor->IsDragging())
                    {
                        itemActor->OnMouseMove(mMousePos);
                    }
                }
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
