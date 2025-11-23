#pragma once
#include <vector>
#include <string>
#include <memory>
#include "../Math.h"
#include "../Core/Texture/Texture.hpp"
#include "TiledParser.hpp"

enum class TileType
{
    Floor,
    Wall,
    Water,
    Grass,
    Path
};

struct Tile
{
    TileType type;
    bool walkable;
    int gid;  // Global tile ID from tileset (for advanced tilesets)
};

// For Tiled JSON format support
struct Layer {
    std::string name;
    int width;
    int height;
    std::vector<int> data;  // GIDs for tiles
};

struct MapData {
    int mapWidth;
    int mapHeight;
    int tileWidth;
    int tileHeight;
    std::vector<TilesetInfo> tilesets;
    std::vector<Layer> layers;
};

class TileMap
{
public:
    TileMap(int width, int height, int tileSize);
    ~TileMap();
    
    // Load from Tiled JSON (optional advanced feature)
    bool LoadFromJSON(const std::string& jsonPath);
    
    // Generate simple procedural map
    void GenerateMap();
    
    // Draw using your OpenGL SpriteRenderer
    void Draw(class SpriteRenderer* spriteRenderer);
    
    // Collision checking
    bool IsWalkable(const Vector2& position) const;
    TileType GetTileAt(const Vector2& position) const;
    
    // Getters
    int GetWidth() const { return mWidth; }
    int GetHeight() const { return mHeight; }
    int GetTileSize() const { return mTileSize; }
    MapData* GetMapData() { return mMapData.get(); }
    
private:
    int mWidth;
    int mHeight;
    int mTileSize;
    std::vector<std::vector<Tile>> mTiles;
    
    // Simple textures for basic tiles
    std::unique_ptr<Texture> mGrassTexture;
    std::unique_ptr<Texture> mWaterTexture;
    std::unique_ptr<Texture> mPathTexture;
    
    // Advanced tileset support (optional)
    std::unique_ptr<MapData> mMapData;
    
    // Helper methods
    Tile CreateTile(TileType type);
};
