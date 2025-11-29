#include "ItemGenerator.hpp"
#include "Game.hpp"
#include "../Map/TileMap.hpp"
#include "../Actor/ItemActor.hpp"
#include <iostream>

ItemGenerator::ItemGenerator(Game* game) : mGame(game) {
    InitializeMappings();
}

void ItemGenerator::InitializeMappings() {
    // Map layer names to item names
    // Format: "layer_name" -> "Item Name"
    mLayerToItemMap["gerador_agua"] = "Água";
    // Future layers can be added here
}

void ItemGenerator::GenerateItemsFromMap(TileMap* tileMap) {
    if (!tileMap || !mGame) return;

    auto mapData = tileMap->GetMapData();
    if (!mapData) return;

    int tileSize = tileMap->GetTileSize();

    for (const auto& layer : mapData->layers) {
        auto it = mLayerToItemMap.find(layer.name);
        if (it != mLayerToItemMap.end()) {
            std::string itemName = it->second;
            
            // Find the item definition
            const Item* itemDef = nullptr;
            if (mGame->GetCrafting()) {
                for (const auto& item : mGame->GetCrafting()->GetAllItems()) {
                    if (item.name == itemName) {
                        itemDef = &item;
                        break;
                    }
                }
            }

            if (!itemDef) {
                std::cerr << "Warning: ItemGenerator could not find item definition for '" << itemName << "'" << std::endl;
                continue;
            }

            int count = 0;
            for (int y = 0; y < layer.height; y++) {
                for (int x = 0; x < layer.width; x++) {
                    int index = y * layer.width + x;
                    if (index < layer.data.size() && layer.data[index] != 0) {
                        // Spawn item
                        auto itemActor = std::make_unique<ItemActor>(mGame, *itemDef);
                        Vector2 pos(x * tileSize + tileSize / 2.0f, y * tileSize + tileSize / 2.0f);
                        itemActor->SetPosition(pos);
                        mGame->AddActor(std::move(itemActor));
                        count++;
                    }
                }
            }
            // std::cout << "ItemGenerator: Spawned " << count << " items of type '" << itemName << "' from layer '" << layer.name << "'" << std::endl;
        }
    }
}
