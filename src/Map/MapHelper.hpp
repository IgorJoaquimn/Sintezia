#pragma once

#include <SDL.h>
#include <vector>
#include <string>
#include <memory>
#include "../Math.h"

// Layer types in the tilemap
enum class LayerType
{
    Block,      // Tile layer with GIDs
    Object      // Object layer with entities
};

// Information about a tileset
struct TilesetInfo {
    int firstGid;           // First global ID
    int tileWidth;          // Width of each tile
    int tileHeight;         // Height of each tile
    std::string imagePath;  // Path to tileset image
    SDL_Texture* texture;   // Loaded texture
    int columns;            // Number of columns in tileset
    int rows;               // Number of rows in tileset
    int tileCount;          // Total number of tiles
    int spacing;            // Spacing between tiles
    int margin;             // Margin around tileset
};

// Dynamic object in the map (NPCs, items, etc.)
struct DynamicObject {
    int id;
    std::string name;
    std::string type;
    Vector2 pos;
    int width;
    int height;
};

// Layer in the map
struct Layer {
    LayerType type;
    std::string name;
    int width;
    int height;
    
    // For Block layers
    std::vector<int> data;  // GIDs for tiles
    
    // For Object layers
    std::vector<DynamicObject> objects;
};

// Complete map data
struct MapData {
    int mapWidth;           // Width in tiles
    int mapHeight;          // Height in tiles
    int tileWidth;          // Width of each tile in pixels
    int tileHeight;         // Height of each tile in pixels
    std::vector<TilesetInfo> tilesets;
    std::vector<Layer> layers;
};

// Tile rendering info with flip flags
struct TileRenderInfo {
    uint32_t cleanGid;      // GID without flip flags
    double angleDeg;        // Rotation angle
    SDL_RendererFlip flip;  // Flip flags
};

// Helper functions
int FindTilesetIndex(const MapData* map, int gid);
TileRenderInfo GetTileFlipInfoFromGID(uint32_t gidWithFlags);
int GetLayerIdx(const MapData& map, const std::string& layerName);
