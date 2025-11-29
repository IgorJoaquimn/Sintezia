#pragma once
#include <string>
#include <vector>
#include <map>

class Game;
class TileMap;

class ItemGenerator {
public:
    ItemGenerator(Game* game);
    void GenerateItemsFromMap(TileMap* tileMap);

private:
    Game* mGame;
    std::map<std::string, std::string> mLayerToItemMap;
    
    void InitializeMappings();
};
