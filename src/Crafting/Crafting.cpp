#include "Crafting.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>

Crafting::Crafting() {
}

Crafting::~Crafting() {
}

std::string Crafting::CreateRecipeKey(int id1, int id2) const {
    // Always use the smaller ID first to ensure consistency
    int minId = std::min(id1, id2);
    int maxId = std::max(id1, id2);
    return std::to_string(minId) + "," + std::to_string(maxId);
}

void Crafting::RegisterRecipe(int item1Id, int item2Id, int resultId) {
    std::string key = CreateRecipeKey(item1Id, item2Id);
    
    // Find the result item from the items list
    const Item* resultItem = FindItemById(resultId);
    if (resultItem) {
        recipes[key] = std::make_unique<Item>(resultItem->id, resultItem->name, resultItem->emoji);
    } else {
        std::cerr << "Warning: Result item with ID " << resultId << " not found!" << std::endl;
    }
}

std::unique_ptr<Item> Crafting::combine_items(const Item& item1, const Item& item2) {
    std::string key = CreateRecipeKey(item1.id, item2.id);
    
    auto it = recipes.find(key);
    if (it != recipes.end()) {
        // Create a copy of the result item
        return std::make_unique<Item>(it->second->id, it->second->name, it->second->emoji);
    }
    
    // No valid recipe found
    return nullptr;
}

bool Crafting::LoadItemsFromJson(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open items file: " << filepath << std::endl;
        return false;
    }
    
    try {
        json j;
        file >> j;
        
        items.clear();
        for (const auto& itemJson : j["items"]) {
            items.push_back(Item::fromJson(itemJson));
        }
        
        std::cout << "Loaded " << items.size() << " items from " << filepath << std::endl;
        return true;
    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return false;
    }
}

bool Crafting::LoadRecipesFromJson(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open recipes file: " << filepath << std::endl;
        return false;
    }
    
    try {
        json j;
        file >> j;
        
        recipes.clear();
        int recipeCount = 0;
        
        for (const auto& recipeJson : j["recipes"]) {
            int item1Id = recipeJson.at("item1_id").get<int>();
            int item2Id = recipeJson.at("item2_id").get<int>();
            int resultId = recipeJson.at("result_id").get<int>();
            
            // Find the result item from loaded items
            const Item* resultItem = FindItemById(resultId);
            if (resultItem) {
                std::string key = CreateRecipeKey(item1Id, item2Id);
                recipes[key] = std::make_unique<Item>(resultItem->id, resultItem->name, resultItem->emoji);
                recipeCount++;
            } else {
                std::cerr << "Warning: Recipe references unknown result item ID: " << resultId << std::endl;
            }
        }
        
        std::cout << "Loaded " << recipeCount << " recipes from " << filepath << std::endl;
        return true;
    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return false;
    }
}

void Crafting::AddItem(const Item& item) {
    items.push_back(item);
}

const Item* Crafting::FindItemById(int id) const {
    for (const auto& item : items) {
        if (item.id == id) {
            return &item;
        }
    }
    return nullptr;
}
