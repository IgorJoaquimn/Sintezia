#pragma once
#include <vector>
#include <string>
#include "../Math.h"

enum class TileType
{
    Grass,
    Water,
    Sand,
    Tree,
    Rock
};

struct Tile
{
    TileType type;
    std::string emoji;
    bool walkable;
};

class TileMap
{
public:
    TileMap(int width, int height, int tileSize);
    ~TileMap();
    
    void GenerateMap();
    void Draw(class TextRenderer* textRenderer);
    
    bool IsWalkable(const Vector2& position) const;
    TileType GetTileAt(const Vector2& position) const;
    
    int GetWidth() const { return mWidth; }
    int GetHeight() const { return mHeight; }
    int GetTileSize() const { return mTileSize; }
    
private:
    int mWidth;
    int mHeight;
    int mTileSize;
    std::vector<std::vector<Tile>> mTiles;
    
    Tile CreateTile(TileType type);
};
