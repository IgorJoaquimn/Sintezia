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

    Item(int id, const std::string& name, const std::string& emoji = "🔹") 
        : id(id), name(name), emoji(emoji) {}
    
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
        {"emoji", item.emoji}
    };
}

inline void from_json(const json& j, Item& item) {
    j.at("id").get_to(item.id);
    j.at("name").get_to(item.name);
    j.at("emoji").get_to(item.emoji);
}

#endif // ITEM_HPP
