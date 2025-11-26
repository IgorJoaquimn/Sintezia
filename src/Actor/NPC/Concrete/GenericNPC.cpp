#include "GenericNPC.hpp"
#include "../../../Game/Game.hpp"
#include "../../../Component/SpriteComponent.hpp"
#include "../../../Component/AnimationComponent.hpp"
#include "../../../Component/MovementComponent.hpp"
#include <iostream>

GenericNPC::GenericNPC(Game* game, const json& npcData)
    : DialogNPC(game)
{
    LoadFromJSON(npcData);
}

void GenericNPC::LoadFromJSON(const json& npcData)
{
    // 1. Basic Info
    if (npcData.contains("name")) {
        // You might want to store the name in the NPC class if needed
        // For now, DialogNPC doesn't seem to have a name field exposed, 
        // but we can assume the greeting or UI uses it.
        // Let's assume we just use it for the greeting for now or add it to DialogNPC later.
    }

    // 2. Position
    if (npcData.contains("position")) {
        float x = npcData["position"]["x"];
        float y = npcData["position"]["y"];
        // Assuming map coordinates, multiply by tile size (40)
        SetPosition(Vector2(x * 40.0f, y * 40.0f));
    }

    // 3. Sprite
    if (npcData.contains("sprite")) {
        SetupSprite(npcData["sprite"]);
    }

    // 4. Faceset
    if (npcData.contains("faceset")) {
        SetupFaceset(npcData["faceset"]);
    }

    // 5. Dialogues (Intro)
    if (npcData.contains("dialogues") && npcData["dialogues"].contains("intro")) {
        std::string greeting = "";
        for (const auto& line : npcData["dialogues"]["intro"]) {
            greeting += line.get<std::string>() + "\n";
        }
        SetGreeting(greeting);
    }

    // 6. Quests (Mapped to TradeOffers for now)
    if (npcData.contains("quests")) {
        for (const auto& quest : npcData["quests"]) {
            std::string title = quest["title"];
            std::string desc = quest["description"];
            std::string startDialog = quest["start_dialogue"];
            std::string endDialog = quest["end_dialogue"];
            int rewardId = quest["reward_item"];
            
            // Create a trade offer
            // Description combines title and description
            std::string tradeDesc = title + ": " + desc;
            
            // Reward quantity is 1 by default
            TradeOffer offer(tradeDesc, rewardId, 1);
            
            // Requirements
            if (quest.contains("required_items")) {
                for (int reqId : quest["required_items"]) {
                    offer.AddRequirement(reqId, 1);
                }
            }
            
            AddTradeOffer(offer);
            
            // Also add a dialog option to ask about the quest
            AddDialogOption("Sobre " + title, startDialog);
        }
    }
}

void GenericNPC::SetupSprite(const std::string& spriteName)
{
    // Construct path based on sprite name
    // Assuming standard path structure from the asset pack
    std::string basePath = "assets/third_party/Ninja Adventure - Asset Pack/Actor/Characters/" + spriteName + "/SpriteSheet.png";
    
    // Load texture using SpriteComponent
    if (mSpriteComponent->LoadSpriteSheet(basePath)) {
        // Configure sprite dimensions (assuming standard 16x16 or similar from the pack)
        // The asset pack usually has 4x7 spritesheets for characters
        // 4 columns (Down, Up, Left, Right) x 1 frames? No, usually 4 frames per direction.
        // Let's check the standard configuration for this pack.
        // Usually 16x16 frames.
        
        // Standard Ninja Adventure pack character spritesheet:
        // 4 directions (Down, Up, Left, Right)
        // 1 frame per direction? Or animated?
        // Actually, looking at the folders, they have "SeparateAnim" and "SpriteSheet.png".
        // Let's assume standard 16x16 frames, 4 columns, 7 rows?
        // Or 4 rows, 4 columns?
        
        // Let's set a default configuration
        SetSpriteConfiguration(16, 16, 1, 4, 10.0f);
        SetIdleRows(0, 2, 3, 1); // Down, Left, Right, Up
        SetWalkRows(0, 2, 3, 1);
        
        // Scale up a bit
        SetScale(Vector2(2.0f, 2.0f));
    } else {
        SDL_Log("Failed to load sprite for NPC: %s", spriteName.c_str());
    }
}

void GenericNPC::SetupFaceset(const std::string& facesetPath)
{
    if (mDialogUI) {
        mDialogUI->SetFacesetTexture(facesetPath);
    }
}
