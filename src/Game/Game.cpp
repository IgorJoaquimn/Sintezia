// ----------------------------------------------------------------
// Game implementation following the asteroids game architecture
// ----------------------------------------------------------------

#include "Game.hpp"
#include "../Actor/Actor.hpp"
#include "../Actor/TextActor.hpp"
#include "../Actor/ItemActor.hpp"
#include "../Actor/Player.hpp"
#include "../UI/InventoryUI.hpp"
#include "../UI/WarningPopup.hpp"
#include "../UI/ControlsUI.hpp"
#include "../UI/GameOverUI.hpp"
#include "../Actor/NPC/Base/DialogNPC.hpp"
#include "../Actor/NPC/Concrete/Aggressive/Skeleton.hpp"
#include "../Actor/NPC/Concrete/Aggressive/Flam.hpp"
#include "../Actor/NPC/Concrete/Aggressive/GoldenStatue.hpp"
#include "../Actor/NPC/Concrete/Aggressive/Shaman.hpp"
#include "../Actor/NPC/Concrete/Aggressive/Spirit.hpp"
#include "../Actor/NPC/Concrete/Aggressive/Statue.hpp"
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
#include <sstream>
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
    , mIsGameOver(false)
    , mIsVictory(false)
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

    // Initialize controls UI
    mControlsUI = std::make_unique<ControlsUI>();

    // Initialize game over UI
    mGameOverUI = std::make_unique<GameOverUI>();

    // Initialize victory UI
    mVictoryUI = std::make_unique<VictoryUI>();

    // Load items and recipes from JSON
    if (!mCrafting->LoadItemsFromJson("assets/items.json"))
    {
        SDL_Log("Warning: Failed to load items");
    }

    if (!mCrafting->LoadRecipesFromJson("assets/recipes.json"))
    {
        SDL_Log("Warning: Failed to load recipes");
    }

    LoadLevel();

    // Set different text color for variety
    mTextRenderer->SetTextColor(1.0f, 1.0f, 1.0f); // White
    mTicksCount = SDL_GetTicks();

    return true;
}

void Game::LoadLevel()
{
    // Create tile map
    // Map is 90x80 tiles (3600x3200 pixels)
    mTileMap = std::make_unique<TileMap>(90, 80, 40);

    // Load your custom Tiled map
    if (!mTileMap->LoadFromJSON("assets/maps/mapa_de_teste.json"))
    {
        SDL_Log("Warning: Failed to load custom map, using procedural generation");
    }

    if (!mAudio) {
        mAudio = new AudioSystem();
        // mAudio->PlaySound("background.ogg", true, 50);
    }

    // Create player
    auto player = std::make_unique<Player>(this);
    mPlayer = player.get(); // Safe: player ownership transferred to mActors, pointer valid for game lifetime
    // Log player spawn position (x, y)
    if (mPlayer) {
        Vector2 ppos = mPlayer->GetPosition();
        SDL_Log("Player spawn position: (%.2f, %.2f)", ppos.x, ppos.y);
    }
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

    // Load enemies from JSON
    LoadEnemiesFromJson("assets/enemies.json");

    // Debug: log current actor count and positions to verify enemies were added
    SDL_Log("Actor count after loading NPCs & enemies: %zu", mActors.size());
}

void Game::RestartGame()
{
    mIsGameOver = false;
    mIsVictory = false;
    mActors.clear();
    mPendingActors.clear();
    mNPCs.clear();
    mInteractingNPC = nullptr;
    
    LoadLevel();
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
                else if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    mMousePos.x = static_cast<float>(event.button.x);
                    mMousePos.y = static_cast<float>(event.button.y);
                    if (mPlayer && mPlayer->GetInventoryUI())
                    {
                        mPlayer->GetInventoryUI()->HandleRightClick(mMousePos);
                    }
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mMousePos.x = static_cast<float>(event.button.x);
                    mMousePos.y = static_cast<float>(event.button.y);
                    if (mPlayer && mPlayer->GetInventoryUI())
                    {
                        mPlayer->GetInventoryUI()->HandleMouseUp(mMousePos);
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

    // Handle Game Over Input
    if (mIsGameOver)
    {
        if (keyState[SDL_SCANCODE_R])
        {
            RestartGame();
        }
        else if (keyState[SDL_SCANCODE_ESCAPE])
        {
            Quit();
        }
        return; // Don't process other inputs
    }

    // Handle Victory Input
    if (mIsVictory)
    {
        if (keyState[SDL_SCANCODE_R])
        {
            RestartGame();
        }
        else if (keyState[SDL_SCANCODE_ESCAPE])
        {
            Quit();
        }
        return; // Don't process other inputs
    }

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

    

    // Check if game is paused (interacting with NPC or Warning Popup or Game Over or Victory)
    bool isPaused = (mInteractingNPC && mInteractingNPC->IsInteracting()) || 
                    (mWarningPopup && mWarningPopup->IsVisible()) ||
                    mIsGameOver || mIsVictory;

    // Update all actors
    mUpdatingActors = true;

    for (auto& actor : mActors)
    {
        // When paused, only update the interacting NPC (for dialog UI)
        if (isPaused)
        {
            if (mInteractingNPC && actor.get() == mInteractingNPC)
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
    if (!isPaused && mPlayer && mTileMap)
    {
        int mapWidth = mTileMap->GetWidth() * mTileMap->GetTileSize();
        int mapHeight = mTileMap->GetHeight() * mTileMap->GetTileSize();

        mCamera->Update(deltaTime, mPlayer->GetPosition(), mapWidth, mapHeight);
    }

    if (!isPaused)
    {
        UpdateComponents(deltaTime);
    }

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

    if (mControlsUI)
    {
        mControlsUI->Draw(mSpriteRenderer.get(), mTextRenderer.get());
    }

    if (mWarningPopup)
    {
        mWarningPopup->Draw(mTextRenderer.get(), mRectRenderer.get(), mSpriteRenderer.get());
    }

    // Draw Game Over UI
    if (mIsGameOver && mGameOverUI)
    {
        mGameOverUI->Draw(mTextRenderer.get(), mRectRenderer.get(), mSpriteRenderer.get());
    }

    // Draw Victory UI
    if (mIsVictory && mVictoryUI)
    {
        mVictoryUI->Draw(mTextRenderer.get(), mRectRenderer.get(), mSpriteRenderer.get());
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

void Game::SetGameOver(bool gameOver)
{
    if (mIsGameOver != gameOver)
    {
        mIsGameOver = gameOver;
        SDL_Log("Game Over state changed to: %s", gameOver ? "TRUE" : "FALSE");
    }
}

void Game::SetVictory(bool victory)
{
    if (mIsVictory != victory)
    {
        mIsVictory = victory;
        SDL_Log("Victory state changed to: %s", victory ? "TRUE" : "FALSE");
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


void Game::LoadEnemiesFromJson(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        SDL_Log("Failed to open enemies file: %s", filePath.c_str());
        return;
    }

    try
    {
        nlohmann::json j;
        file >> j;

        if (!j.contains("enemies")) return;

        int loadedCount = 0;

        for (const auto& e : j["enemies"])
        {
            std::string type = e.value("type", "");
            int x = e.value("x", 0);
            int y = e.value("y", 0);

            std::unique_ptr<Actor> enemyActor;

            if (type == "Flam")
            {
                auto ptr = std::make_unique<FlamNPC>(this);
                ptr->SetGridPosition(x, y);
                ptr->SetAnchorPosition(ptr->GetPosition());
                enemyActor = std::move(ptr);
            }
            else if (type == "GoldenStatue")
            {
                auto ptr = std::make_unique<GoldenStatueNPC>(this);
                ptr->SetGridPosition(x, y);
                ptr->SetAnchorPosition(ptr->GetPosition());
                enemyActor = std::move(ptr);
            }
            else if (type == "Shaman")
            {
                auto ptr = std::make_unique<ShamanNPC>(this);
                ptr->SetGridPosition(x, y);
                ptr->SetAnchorPosition(ptr->GetPosition());
                enemyActor = std::move(ptr);
            }
            else if (type == "Spirit")
            {
                auto ptr = std::make_unique<SpiritNPC>(this);
                ptr->SetGridPosition(x, y);
                ptr->SetAnchorPosition(ptr->GetPosition());
                enemyActor = std::move(ptr);
            }
            else if (type == "Statue")
            {
                auto ptr = std::make_unique<StatueNPC>(this);
                ptr->SetGridPosition(x, y);
                ptr->SetAnchorPosition(ptr->GetPosition());
                enemyActor = std::move(ptr);
            }
            else if (type == "Skeleton")
            {
                auto ptr = std::make_unique<SkeletonNPC>(this);
                ptr->SetGridPosition(x, y);
                ptr->SetAnchorPosition(ptr->GetPosition());
                enemyActor = std::move(ptr);
            }
            else if (type == "Pig")
            {
                auto ptr = std::make_unique<PigNPC>(this);
                ptr->SetGridPosition(x, y);
                enemyActor = std::move(ptr);
            }
            else if (type == "Cow")
            {
                auto ptr = std::make_unique<CowNPC>(this);
                ptr->SetGridPosition(x, y);
                enemyActor = std::move(ptr);
            }
            else if (type == "Chicken")
            {
                auto ptr = std::make_unique<ChickenNPC>(this);
                ptr->SetGridPosition(x, y);
                enemyActor = std::move(ptr);
            }
            else
            {
                SDL_Log("Unknown enemy type: %s", type.c_str());
                continue;
            }

            // Optional overrides
            if (e.contains("movementSpeed"))
            {
                auto* patrol = dynamic_cast<PatrolNPC*>(enemyActor.get());
                if (patrol)
                {
                    patrol->SetMovementSpeed(e["movementSpeed"].get<float>());
                }
            }

            if (e.contains("chaseSpeed"))
            {
                // PatrolNPC exposes SetChaseSpeed but we need to cast
                auto* patrol = dynamic_cast<PatrolNPC*>(enemyActor.get());
                if (patrol) patrol->SetChaseSpeed(e["chaseSpeed"].get<float>());
            }

            if (e.contains("aggroRange"))
            {
                auto* patrol = dynamic_cast<PatrolNPC*>(enemyActor.get());
                if (patrol) patrol->SetAggroRange(e["aggroRange"].get<float>());
            }

            if (e.contains("deaggroRange"))
            {
                auto* patrol = dynamic_cast<PatrolNPC*>(enemyActor.get());
                if (patrol) patrol->SetDeaggroRange(e["deaggroRange"].get<float>());
            }

            if (e.contains("maxChaseDistance"))
            {
                auto* patrol = dynamic_cast<PatrolNPC*>(enemyActor.get());
                if (patrol) patrol->SetMaxChaseDistance(e["maxChaseDistance"].get<float>());
            }

            // Optional waypoints
            if (e.contains("waypoints") && enemyActor)
            {
                PatrolNPC* patrol = dynamic_cast<PatrolNPC*>(enemyActor.get());
                if (patrol)
                {
                    for (const auto& wp : e["waypoints"])
                    {
                        float wx = wp.value("x", 0.0f);
                        float wy = wp.value("y", 0.0f);
                        float wait = wp.value("wait", 0.0f);
                        patrol->AddWaypoint(Vector2(wx, wy), wait);
                    }
                }
            }

            // Finally add actor to game
            if (enemyActor)
            {
                // Build a log message with details about loaded enemy
                int waypointCount = e.contains("waypoints") ? static_cast<int>(e["waypoints"].size()) : 0;
                float mv = e.contains("movementSpeed") ? e["movementSpeed"].get<float>() : -1.0f;
                float ch = e.contains("chaseSpeed") ? e["chaseSpeed"].get<float>() : -1.0f;
                float ag = e.contains("aggroRange") ? e["aggroRange"].get<float>() : -1.0f;
                float de = e.contains("deaggroRange") ? e["deaggroRange"].get<float>() : -1.0f;
                float mx = e.contains("maxChaseDistance") ? e["maxChaseDistance"].get<float>() : -1.0f;

                std::ostringstream oss;
                oss << "Loaded enemy type=" << type << " pos=(" << x << "," << y << ")";
                if (mv >= 0.0f) oss << " movementSpeed=" << mv;
                if (ch >= 0.0f) oss << " chaseSpeed=" << ch;
                if (ag >= 0.0f) oss << " aggroRange=" << ag;
                if (de >= 0.0f) oss << " deaggroRange=" << de;
                if (mx >= 0.0f) oss << " maxChaseDistance=" << mx;
                oss << " waypoints=" << waypointCount;

                SDL_Log("%s", oss.str().c_str());

                ++loadedCount;
                AddActor(std::move(enemyActor));
            }
        }

        SDL_Log("Loaded %d enemies from %s", loadedCount, filePath.c_str());
    }
    catch (const std::exception& ex)
    {
        SDL_Log("Error parsing enemies JSON: %s", ex.what());
    }
}

void Game::UpdateComponents(float deltaTime)
{
    for (auto& actor : mActors)
    {
        auto health = actor->GetComponent<HealthComponent>();
        auto hunger = actor->GetComponent<HungerComponent>();
        auto thirst = actor->GetComponent<ThirstComponent>();


        if (health && hunger && thirst)
        {
            float hungerRatio = hunger->GetCurrentHunger() / hunger->GetMaxHunger();
            float thirstRatio = thirst->GetCurrentThirst() / thirst->GetMaxThirst();
            health->UpdateVitalityBar(hungerRatio, thirstRatio);
        }
    }
}