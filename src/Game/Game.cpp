// ----------------------------------------------------------------
// Game implementation following the asteroids game architecture
// ----------------------------------------------------------------

#include "Game.hpp"
#include "../Actor/Actor.hpp"
#include "../Actor/TextActor.hpp"
#include "../Actor/ItemActor.hpp"
#include "../Actor/Player.hpp"
#include "../UI/InventoryUI.hpp"
#include "../Actor/NPC/Base/DialogNPC.hpp"
#include "../Actor/NPC/Concrete/Aggressive/Skeleton.hpp"
#include "../Actor/NPC/Concrete/Passive/CatNPC.hpp"
#include "../Actor/NPC/Concrete/Passive/CowNPC.hpp"
#include "../Actor/NPC/Concrete/Passive/ChickenNPC.hpp"
#include "../Actor/NPC/Concrete/Passive/PigNPC.hpp"
#include "../Map/TileMap.hpp"
#include "../Core/Renderer/Renderer.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include "../Core/RenderUtils.hpp"
#include "../Crafting/Crafting.hpp"
#include "Inventory.hpp"
#include "ItemGenerator.hpp"
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include "../Actor/NPC/Concrete/Passive/GenericNPC.hpp"
#include "../Component/MovementComponent.hpp"
#include "../Component/HealthComponent.hpp"
#include "../Component/HungerComponent.hpp"
#include "../Component/ThirstComponent.hpp"

Game::Game(SDL_Window* window, SDL_GLContext glContext)
    : mWindow(window)
    , mGLContext(glContext)
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
    , mCamera(std::make_unique<Camera>(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)))
    , mAudio(nullptr)
{
}

Game::~Game() = default;

bool Game::Initialize()
{
    // SDL and OpenGL context must be initialized before calling this!
    // Remove SDL_Init, SDL_GL_SetAttribute, SDL_CreateWindow, SDL_GL_CreateContext
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

    // Initialize warning popup
    mWarningPopup = std::make_unique<WarningPopup>();

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
    // Map is 90x80 tiles (3600x3200 pixels)
    mTileMap = std::make_unique<TileMap>(90, 80, 40);

    // Load your custom Tiled map
    if (!mTileMap->LoadFromJSON("assets/maps/mapa_de_teste.json"))
    {
        SDL_Log("Warning: Failed to load custom map, using procedural generation");
    }

    mAudio = new AudioSystem();

    if (mAudio) {
        mAudio->PlaySound("background.ogg", true, 50);
    }

    // NOTE: Items are no longer spawned automatically. They are generated
    // when the player attacks resource blocks (gerador_agua, gerador_fogo, gerador_madeira)
    // This is handled in AttackComponent::TryHarvestResource()
    // ItemGenerator itemGenerator(this);
    // itemGenerator.GenerateItemsFromMap(mTileMap.get());

    // Create player
    auto player = std::make_unique<Player>(this);
    mPlayer = player.get(); // Safe: player ownership transferred to mActors, pointer valid for game lifetime
    AddActor(std::move(player));

    // Update player movement bounds to match map size
    if (mPlayer && mTileMap)
    {
        float mapWidth = static_cast<float>(mTileMap->GetWidth() * mTileMap->GetTileSize());
        float mapHeight = static_cast<float>(mTileMap->GetHeight() * mTileMap->GetTileSize());

        auto* moveComp = mPlayer->GetComponent<MovementComponent>();
        if (moveComp)
        {
            // Keep a small margin from the absolute edge
            moveComp->SetBounds(16.0f, 16.0f, mapWidth - 16.0f, mapHeight - 16.0f);
        }

        // Snap camera to player start position
        mCamera->SnapToPlayer(mPlayer->GetPosition(), static_cast<int>(mapWidth), static_cast<int>(mapHeight));
    }

    // Give player some starting items for testing trades
    if (mPlayer && mPlayer->GetInventory() && mCrafting)
    {
        // Add some basic elements that the shopkeeper wants
        const Item* water = mCrafting->FindItemById(1);  // Water
        const Item* fire = mCrafting->FindItemById(2);   // Fire
        const Item* earth = mCrafting->FindItemById(3);  // Earth

        if (water)
            mPlayer->GetInventory()->AddItem(*water, 5);  // 5 water
        if (fire)
            mPlayer->GetInventory()->AddItem(*fire, 5);   // 5 fire
        if (earth)
            mPlayer->GetInventory()->AddItem(*earth, 3);  // 3 earth

                    // SDL_Log("Added starting items to player inventory");
    }

    // Create test aggressive patrol NPC (patrols and chases player)
    auto skeletonNPC = std::make_unique<SkeletonNPC>(this);
    AddActor(std::move(skeletonNPC));

    // Create cat NPC (friendly dialog NPC with simple animation)
    auto catNPC = std::make_unique<CatNPC>(this);
    AddActor(std::move(catNPC));

    // Create farm animals
    auto cowNPC = std::make_unique<CowNPC>(this);
    AddActor(std::move(cowNPC));

    auto chickenNPC = std::make_unique<ChickenNPC>(this);
    AddActor(std::move(chickenNPC));

    auto pigNPC = std::make_unique<PigNPC>(this);
    AddActor(std::move(pigNPC));

    // Load NPCs from JSON
    LoadNPCsFromJson("assets/npcs.json");

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
            case SDL_MOUSEMOTION:
                mMousePos.x = static_cast<float>(event.motion.x);
                mMousePos.y = static_cast<float>(event.motion.y);
                if (mPlayer && mPlayer->GetInventoryUI())
                {
                    mPlayer->GetInventoryUI()->HandleMouseMove(mMousePos);
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mMousePos.x = static_cast<float>(event.button.x);
                    mMousePos.y = static_cast<float>(event.button.y);
                    if (mPlayer && mPlayer->GetInventoryUI())
                    {
                        mPlayer->GetInventoryUI()->HandleMouseClick(mMousePos);
                    }
                }
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
        static bool spaceKeyPressed = false;

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

        // Check for SPACE key press to interact with nearby NPCs
        bool startedInteraction = false;
        if (keyState[SDL_SCANCODE_SPACE] && !spaceKeyPressed)
        {
            spaceKeyPressed = true;

            if (nearbyNPC)
            {
                nearbyNPC->StartInteraction();
                mInteractingNPC = nearbyNPC;
                startedInteraction = true;

                // Stop player movement when starting interaction
                if (mPlayer)
                {
                    mPlayer->StopMovement();
                }
            }
        }
        else if (!keyState[SDL_SCANCODE_SPACE])
        {
            spaceKeyPressed = false;
        }

        // Process player input only when not interacting (and didn't just start interaction this frame)
        if (mPlayer && !startedInteraction)
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
        SDL_Delay(1);
    }

    float deltaTime = static_cast<float>(SDL_GetTicks() - mTicksCount) / 1000.0f;
    if (deltaTime > 0.05f)
    {
        deltaTime = 0.05f;
    }

    mTicksCount = SDL_GetTicks();

    

    // Check if game is paused (interacting with NPC)
    bool isPaused = mInteractingNPC && mInteractingNPC->IsInteracting();

    // Update all actors
    mUpdatingActors = true;

    for (auto& actor : mActors)
    {
        // When paused, only update the interacting NPC (for dialog UI)
        if (isPaused)
        {
            if (actor.get() == mInteractingNPC)
            {
                actor->Update(deltaTime);
            }
        }
        else
        {
            actor->Update(deltaTime);
        }
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

    // Update camera position to follow player
    if (mPlayer && mTileMap)
    {
        int mapWidth = mTileMap->GetWidth() * mTileMap->GetTileSize();
        int mapHeight = mTileMap->GetHeight() * mTileMap->GetTileSize();

        mCamera->Update(deltaTime, mPlayer->GetPosition(), mapWidth, mapHeight);
    }

    UpdateComponents(deltaTime);

    if (mWarningPopup)
    {
        mWarningPopup->Update(deltaTime);
    }
}

void Game::GenerateOutput()
{
    mRenderer->BeginFrame();

    // Use common render utility for screen clearing
    RenderUtils::ClearScreen(0.2f, 0.5f, 0.3f, 1.0f); // Green-ish background

    // Update sprite renderer camera
    if (mSpriteRenderer)
    {
        mSpriteRenderer->SetCameraPosition(mCamera->GetPosition());
    }

    // Draw tilemap first
    if (mTileMap)
    {
        mTileMap->Draw(mSpriteRenderer.get());
    }

    // Create a list of active actors to render
    std::vector<Actor*> activeActors;
    activeActors.reserve(mActors.size());

    for (auto& actor : mActors)
    {
        if (actor->GetState() == ActorState::Active)
        {
            activeActors.push_back(actor.get());
        }
    }

    // Sort actors by Y position (ascending) for depth sorting
    // Lower Y (top of screen) drawn first (behind)
    // Higher Y (bottom of screen) drawn last (in front)
    std::sort(activeActors.begin(), activeActors.end(), [](Actor* a, Actor* b) {
        return a->GetPosition().y < b->GetPosition().y;
    });

    // Render sorted actors
    for (auto* actor : activeActors)
    {
        // For TextRenderer, we might need to adjust position manually if it doesn't use SpriteRenderer
        // But TextRenderer usually renders UI or world text.
        // If it's world text, it needs camera offset.
        // Let's assume TextRenderer handles UI (screen space) for now, or check if it needs update.
        // The current TextRenderer implementation likely uses screen coordinates.
        // If actors draw sprites via SpriteComponent, they use SpriteRenderer which now has camera.
        // If they draw text via TextRenderer, we might need to offset.

        actor->OnDraw(mTextRenderer.get());
    }

    // Reset camera for UI rendering
    if (mSpriteRenderer)
    {
        mSpriteRenderer->SetCameraPosition(Vector2::Zero);
    }

    // Render Player UI on top of everything
    if (mPlayer && mPlayer->GetInventoryUI())
    {
        mPlayer->GetInventoryUI()->Draw(mTextRenderer.get(), mRectRenderer.get(), mSpriteRenderer.get());
    }

    if (mWarningPopup)
    {
        mWarningPopup->Draw(mTextRenderer.get(), mRectRenderer.get(), mSpriteRenderer.get());
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

void Game::ShowWarning(const std::string& message)
{
    if (mWarningPopup)
    {
        mWarningPopup->Show(message);
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

        // SDL_Log("Combined %s + %s = %s",
        //         item1->GetItem().name.c_str(),
        //         item2->GetItem().name.c_str(),
        //         result->name.c_str());
    }
}

void Game::LoadNPCsFromJson(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        SDL_Log("Failed to open NPC file: %s", filePath.c_str());
        return;
    }

    try
    {
        nlohmann::json j;
        file >> j;

        if (j.contains("npcs"))
        {
            for (const auto& npcData : j["npcs"])
            {
                auto npc = std::make_unique<GenericNPC>(this, npcData);
                RegisterNPC(npc.get());
                AddActor(std::move(npc));
            }
            // SDL_Log("Loaded NPCs from %s", filePath.c_str());
        }
    }
    catch (const std::exception& e)
    {
        SDL_Log("Error parsing NPC JSON: %s", e.what());
    }
}

void Game::UpdateComponents(float deltaTime)
{
    for (auto& actor : mActors)
    {
        auto health = actor->GetComponent<HealthComponent>();
        auto hunger = actor->GetComponent<HungerComponent>();
        auto thirst = actor->GetComponent<ThirstComponent>();

        if (hunger)
        {
            // Hunger updates itself in its own Update method called by Actor::Update
            // But here we might want to do global game logic if needed
            // hunger->Update(deltaTime); 
        }

        if (thirst)
        {
            // Thirst updates itself in its own Update method called by Actor::Update
            // thirst->IncreaseThirst(deltaTime * 0.1f); 
        }

        if (health && hunger && thirst)
        {
            float hungerRatio = hunger->GetCurrentHunger() / hunger->GetMaxHunger();
            float thirstRatio = thirst->GetCurrentThirst() / thirst->GetMaxThirst();
            health->UpdateVitalityBar(hungerRatio, thirstRatio);
        }
    }
}