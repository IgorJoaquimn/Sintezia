#ifndef CRAFTING_HPP
#define CRAFTING_HPP

#include "Item.hpp"
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Crafting {
public:
    Crafting();
    ~Crafting();

    // Combine two items to create a new item
    // Returns nullptr if combination is not valid
    std::unique_ptr<Item> combine_items(const Item& item1, const Item& item2);

    // Register a crafting recipe
    void RegisterRecipe(int item1Id, int item2Id, int resultId);
    
    // Load items from JSON file
    bool LoadItemsFromJson(const std::string& filepath);
    
    // Load recipes from JSON file
    bool LoadRecipesFromJson(const std::string& filepath);
    
    // Get all loaded items
    const std::vector<Item>& GetAllItems() const { return items; }
    
    // Add an item to the collection
    void AddItem(const Item& item);
    
    // Find item by ID
    const Item* FindItemById(int id) const;

private:
    // Store crafting recipes as a map of (item1_id, item2_id) -> result_id
    // Using a string key for simplicity: "id1,id2" or "id2,id1"
    std::unordered_map<std::string, std::unique_ptr<Item>> recipes;
    
    // Store all items
    std::vector<Item> items;

    // Helper to create recipe key
    std::string CreateRecipeKey(int id1, int id2) const;
};

#endif // CRAFTING_HPP
