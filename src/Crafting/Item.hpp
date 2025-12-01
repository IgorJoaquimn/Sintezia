#ifndef ITEM_HPP
#define ITEM_HPP
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Item {
public:
    int id;
    std::string name;
    std::string emoji;
    float hungerRestoration;
    float thirstRestoration;

    Item(int id, const std::string& name, const std::string& emoji = "🔹", float hungerRestoration = 0.0f, float thirstRestoration = 0.0f) 
        : id(id), name(name), emoji(emoji), hungerRestoration(hungerRestoration), thirstRestoration(thirstRestoration) {}
    
    // JSON serialization
    json toJson() const;
    
    // JSON deserialization
    static Item fromJson(const json& j);
};

// JSON conversion helpers for nlohmann/json library
inline void to_json(json& j, const Item& item) {
    j = json{
        {"id", item.id},
        {"name", item.name},
        {"emoji", item.emoji},
        {"hungerRestoration", item.hungerRestoration},
        {"thirstRestoration", item.thirstRestoration}
    };
}

inline void from_json(const json& j, Item& item) {
    j.at("id").get_to(item.id);
    j.at("name").get_to(item.name);
    j.at("emoji").get_to(item.emoji);
}

#endif // ITEM_HPP
