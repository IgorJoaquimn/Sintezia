#include "Item.hpp"

json Item::toJson() const {
    json j;
    to_json(j, *this);
    return j;
}

Item Item::fromJson(const json& j) {
    return Item(
        j.at("id").get<int>(),
        j.at("name").get<std::string>(),
        j.value("emoji", "🔹"),  // Default emoji if not present
        j.value("hungerRestoration", 0.0f),
        j.value("thirstRestoration", 0.0f)
    );
}
