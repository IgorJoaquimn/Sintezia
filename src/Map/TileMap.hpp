#pragma once
#include <vector>
#include <string>
#include <memory>
#include <map>
#include "../MathUtils.h"
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
    
    // Draw using your OpenGL SpriteRenderer with offset and scale
    void Draw(class SpriteRenderer* spriteRenderer, const Vector2& position, float scale = 1.0f);
    
    // Draw visible tiles directly (culling)
    void DrawVisible(class SpriteRenderer* spriteRenderer, const Vector2& cameraPos, const Vector2& screenSize);
    
    // Get GID from a specific layer (useful for finding UI elements)
    int GetGIDFromLayer(const std::string& layerName);
    
    // Draw a specific GID at a position
    void DrawGID(class SpriteRenderer* spriteRenderer, int gid, const Vector2& position, float scale = 1.0f);
    
    // Collision checking
    bool IsWalkable(const Vector2& position) const;
    bool CheckCollision(const Vector2& position, float radius) const;
    TileType GetTileAt(const Vector2& position) const;
    
    // Getters
    int GetWidth() const { return mWidth; }
    int GetHeight() const { return mHeight; }
    int GetTileSize() const { return mTileSize; }
    MapData* GetMapData() { return mMapData.get(); }
    
    // Resource block harvest tracking
    bool CanHarvestBlock(int col, int row, float currentTime, float cooldown = 5.0f) const;
    void SetBlockHarvestTime(int col, int row, float harvestTime);
    
private:
    int mWidth;
    int mHeight;
    int mTileSize;
    std::vector<std::vector<Tile>> mTiles;
    std::unique_ptr<MapData> mMapData;
    Tile CreateTile(TileType type);
    
    // Track harvest cooldown for resource blocks
    // Key: "col,row", Value: last harvest timestamp
    mutable std::map<std::string, float> mBlockHarvestTimes;

    // Cached rendering
    std::unique_ptr<Texture> mCachedMapTexture;
    GLuint mMapFBO = 0;
    void CacheMap(class SpriteRenderer* spriteRenderer);
};
