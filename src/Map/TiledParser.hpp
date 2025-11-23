#pragma once
#include <string>
#include <memory>
#include <vector>
#include "../Core/Texture/Texture.hpp"

// Tileset information from Tiled
struct TilesetInfo {
    int firstGid;
    int tileWidth;
    int tileHeight;
    std::string imagePath;
    std::unique_ptr<Texture> texture;
    int columns;
    int rows;
    int tileCount;
    int spacing;
    int margin;
    int offsetX = 0;  // Tile rendering offset X
    int offsetY = 0;  // Tile rendering offset Y
    std::vector<bool> tileCollisions; // Per-tile collision (true = blocks movement)
};

// Parser for Tiled TSX (tileset) files
class TiledParser
{
public:
    TiledParser() = default;
    ~TiledParser() = default;
    
    // Parse a TSX file and populate tileset info
    static bool ParseTSX(const std::string& tsxPath, TilesetInfo& tileset);
    
private:
    // Helper to extract XML attributes
    static std::string ExtractAttribute(const std::string& line, const std::string& attrName);
};
