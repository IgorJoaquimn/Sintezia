#include "TileMap.hpp"
#include "TiledParser.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

TileMap::TileMap(int width, int height, int tileSize)
    : mWidth(width)
    , mHeight(height)
    , mTileSize(tileSize)
    , mGrassTexture(std::make_unique<Texture>())
    , mWaterTexture(std::make_unique<Texture>())
    , mPathTexture(std::make_unique<Texture>())
{
    // Load the Cute Fantasy tiles
    if (!mGrassTexture->Load("assets/third_party/Cute_Fantasy_Free/Tiles/Grass_Middle.png"))
    {
        std::cerr << "Failed to load grass texture! Trying fallback..." << std::endl;
        // Try fallback
        if (!mGrassTexture->Load("assets/sprites/Floor.png"))
        {
            std::cerr << "Failed to load floor texture fallback!" << std::endl;
        }
    }
    
    if (!mWaterTexture->Load("assets/third_party/Cute_Fantasy_Free/Tiles/Water_Middle.png"))
    {
        std::cerr << "Failed to load water texture! Trying fallback..." << std::endl;
        if (!mWaterTexture->Load("assets/sprites/Wall.png"))
        {
            std::cerr << "Failed to load wall texture fallback!" << std::endl;
        }
    }
    
    if (!mPathTexture->Load("assets/third_party/Cute_Fantasy_Free/Tiles/Path_Middle.png"))
    {
        std::cerr << "Failed to load path texture!" << std::endl;
    }
    
    GenerateMap();
}

TileMap::~TileMap()
{
}

Tile TileMap::CreateTile(TileType type)
{
    Tile tile;
    tile.type = type;
    tile.gid = 0;
    
    switch (type)
    {
        case TileType::Floor:
        case TileType::Grass:
        case TileType::Path:
            tile.walkable = true;
            break;
        case TileType::Wall:
        case TileType::Water:
            tile.walkable = false;
            break;
    }
    
    return tile;
}

void TileMap::GenerateMap()
{
    mTiles.resize(mHeight);
    
    std::random_device rd;
    std::mt19937 gen(42); // Fixed seed for consistency
    std::uniform_int_distribution<> dis(0, 100);
    
    for (int y = 0; y < mHeight; y++)
    {
        mTiles[y].resize(mWidth);
        for (int x = 0; x < mWidth; x++)
        {
            // Create borders with water
            if (x == 0 || x == mWidth - 1 || y == 0 || y == mHeight - 1)
            {
                mTiles[y][x] = CreateTile(TileType::Water);
            }
            else
            {
                int rand = dis(gen);
                // 10% chance of water tiles inside
                if (rand < 10)
                {
                    mTiles[y][x] = CreateTile(TileType::Water);
                }
                // 10% chance of path tiles
                else if (rand < 20)
                {
                    mTiles[y][x] = CreateTile(TileType::Path);
                }
                else
                {
                    // Rest is grass
                    mTiles[y][x] = CreateTile(TileType::Grass);
                }
            }
        }
    }
}

bool TileMap::LoadFromJSON(const std::string& jsonPath)
{
    std::ifstream file(jsonPath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open tilemap JSON: " << jsonPath << std::endl;
        return false;
    }

    try
    {
        json j;
        file >> j;
        
        mMapData = std::make_unique<MapData>();
        mMapData->mapWidth = j["width"];
        mMapData->mapHeight = j["height"];
        mMapData->tileWidth = j["tilewidth"];
        mMapData->tileHeight = j["tileheight"];
        
        // Load tilesets
        for (const auto& tsJson : j["tilesets"])
        {
            TilesetInfo ts;
            ts.firstGid = tsJson["firstgid"];
            
            // Check if this is an external tileset
            if (tsJson.contains("source"))
            {
                // External tileset - load the .tsx file
                std::string tsxPath = tsJson["source"];
                
                // Resolve path relative to the map file
                std::string basePath = jsonPath.substr(0, jsonPath.find_last_of("/\\") + 1);
                std::string fullTsxPath = basePath + tsxPath;
                
                // Load TSX file using TiledParser
                if (!TiledParser::ParseTSX(fullTsxPath, ts))
                {
                    std::cerr << "Failed to load external tileset: " << fullTsxPath << std::endl;
                    continue;
                }
                
                mMapData->tilesets.push_back(std::move(ts));
                continue;
            }
            
            // Embedded tileset
            ts.tileWidth = tsJson.value("tilewidth", 16);
            ts.tileHeight = tsJson.value("tileheight", 16);
            ts.spacing = tsJson.value("spacing", 0);
            ts.margin = tsJson.value("margin", 0);
            ts.tileCount = tsJson.value("tilecount", 0);
            ts.columns = tsJson.value("columns", 1);
            
            // Get image path
            std::string imagePath = tsJson["image"];
            if (imagePath.find("../") == 0)
            {
                imagePath = imagePath.substr(3);
            }
            ts.imagePath = "assets/" + imagePath;
            
            // Load texture using OpenGL Texture class
            ts.texture = std::make_unique<Texture>();
            if (!ts.texture->Load(ts.imagePath))
            {
                std::cerr << "Failed to load tileset image: " << ts.imagePath << std::endl;
                continue;
            }
            
            // Calculate rows if not specified
            if (ts.tileCount > 0 && ts.columns > 0)
            {
                ts.rows = (ts.tileCount + ts.columns - 1) / ts.columns;
            }
            
            mMapData->tilesets.push_back(std::move(ts));
        }
        
        // Load layers
        for (const auto& layerJson : j["layers"])
        {
            Layer layer;
            layer.name = layerJson["name"];
            layer.width = layerJson["width"];
            layer.height = layerJson["height"];
            
            std::string typeStr = layerJson["type"];
            if (typeStr == "tilelayer" && layerJson.contains("data"))
            {
                for (int gid : layerJson["data"])
                {
                    layer.data.push_back(gid);
                }
            }
            
            mMapData->layers.push_back(layer);
        }
        
        std::cout << "Successfully loaded tilemap: " << jsonPath << std::endl;
        std::cout << "  Map size: " << mMapData->mapWidth << "x" << mMapData->mapHeight << std::endl;
        std::cout << "  Tilesets: " << mMapData->tilesets.size() << std::endl;
        std::cout << "  Layers: " << mMapData->layers.size() << std::endl;
        
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error parsing tilemap JSON: " << e.what() << std::endl;
        return false;
    }
}

void TileMap::Draw(SpriteRenderer* spriteRenderer)
{
    if (!spriteRenderer) return;
    
    // If we have loaded map data from Tiled, draw that instead
    if (mMapData && !mMapData->layers.empty() && !mMapData->tilesets.empty())
    {
        static bool debugPrinted = false;
        if (!debugPrinted)
        {
            std::cout << "Drawing Tiled map:" << std::endl;
            std::cout << "  Map tile size (grid): " << mMapData->tileWidth << "x" << mMapData->tileHeight << std::endl;
            std::cout << "  Display scale for base tiles: " << mTileSize << "px" << std::endl;
            std::cout << "  Map dimensions: " << mMapData->mapWidth << "x" << mMapData->mapHeight << " tiles" << std::endl;
            for (const auto& ts : mMapData->tilesets)
            {
                std::cout << "  Tileset: " << ts.tileWidth << "x" << ts.tileHeight 
                          << "px (" << ts.columns << " cols, " << ts.tileCount << " tiles)" << std::endl;
            }
            debugPrinted = true;
        }
        
        // Calculate base scale factor from map's base tile size to display size
        // This is the scale for tiles that match the map's base tile size
        float baseScale = static_cast<float>(mTileSize) / static_cast<float>(mMapData->tileWidth);
        
        // Draw each layer
        for (const auto& layer : mMapData->layers)
        {
            if (layer.data.empty()) continue;
            
            // Draw each tile in the layer
            for (int y = 0; y < layer.height; y++)
            {
                for (int x = 0; x < layer.width; x++)
                {
                    int index = y * layer.width + x;
                    if (index >= layer.data.size()) continue;
                    
                    int gid = layer.data[index];
                    if (gid == 0) continue; // 0 = empty tile
                    
                    // Find the tileset that contains this GID
                    TilesetInfo* tileset = nullptr;
                    for (auto& ts : mMapData->tilesets)
                    {
                        if (gid >= ts.firstGid && gid < ts.firstGid + ts.tileCount)
                        {
                            tileset = &ts;
                            break;
                        }
                    }
                    
                    if (!tileset || !tileset->texture) continue;
                    
                    // Calculate local tile ID within the tileset
                    int localId = gid - tileset->firstGid;
                    
                    // Calculate source position in the tileset (in pixels)
                    int tileCol = localId % tileset->columns;
                    int tileRow = localId / tileset->columns;
                    
                    float srcX = tileCol * tileset->tileWidth;
                    float srcY = tileRow * tileset->tileHeight;
                    
                    // Get texture dimensions to normalize coordinates
                    int texWidth = tileset->texture->GetWidth();
                    int texHeight = tileset->texture->GetHeight();
                    
                    // Normalize to 0.0-1.0 range for shader
                    float normalizedSrcX = srcX / static_cast<float>(texWidth);
                    float normalizedSrcY = srcY / static_cast<float>(texHeight);
                    float normalizedWidth = tileset->tileWidth / static_cast<float>(texWidth);
                    float normalizedHeight = tileset->tileHeight / static_cast<float>(texHeight);
                    
                    // Calculate destination position on screen
                    // Each grid cell is mTileSize, but tiles can be different sizes
                    float destX = x * mTileSize;
                    float destY = y * mTileSize;
                    
                    // Calculate display size based on the tileset's tile size
                    // Scale proportionally: if tileset tile is 32px and base is 16px,
                    // display at 2x the base display size
                    float scaleFactorX = static_cast<float>(tileset->tileWidth) / static_cast<float>(mMapData->tileWidth);
                    float scaleFactorY = static_cast<float>(tileset->tileHeight) / static_cast<float>(mMapData->tileHeight);
                    
                    float displayWidth = mTileSize * scaleFactorX;
                    float displayHeight = mTileSize * scaleFactorY;
                    
                    // Draw the tile using sprite sheet rendering with normalized coordinates
                    spriteRenderer->DrawSprite(
                        tileset->texture.get(),
                        Vector2(destX, destY),
                        Vector2(displayWidth, displayHeight),
                        Vector2(normalizedSrcX, normalizedSrcY),
                        Vector2(normalizedWidth, normalizedHeight)
                    );
                }
            }
        }
        
        return;
    }
    
    // Otherwise, draw simple procedural tilemap
    for (int y = 0; y < mHeight; y++)
    {
        for (int x = 0; x < mWidth; x++)
        {
            float xPos = x * mTileSize;
            float yPos = y * mTileSize;
            
            Texture* texture = nullptr;
            
            switch (mTiles[y][x].type)
            {
                case TileType::Grass:
                case TileType::Floor:
                    texture = mGrassTexture.get();
                    break;
                case TileType::Water:
                case TileType::Wall:
                    texture = mWaterTexture.get();
                    break;
                case TileType::Path:
                    texture = mPathTexture.get();
                    break;
            }
            
            if (texture)
            {
                spriteRenderer->DrawSprite(
                    texture,
                    Vector2(xPos, yPos),
                    Vector2(mTileSize, mTileSize)
                );
            }
        }
    }
}

bool TileMap::IsWalkable(const Vector2& position) const
{
    int tileX = static_cast<int>(position.x) / mTileSize;
    int tileY = static_cast<int>(position.y) / mTileSize;
    
    if (tileX < 0 || tileX >= mWidth || tileY < 0 || tileY >= mHeight)
        return false;
    
    return mTiles[tileY][tileX].walkable;
}

TileType TileMap::GetTileAt(const Vector2& position) const
{
    int tileX = static_cast<int>(position.x) / mTileSize;
    int tileY = static_cast<int>(position.y) / mTileSize;
    
    if (tileX < 0 || tileX >= mWidth || tileY < 0 || tileY >= mHeight)
        return TileType::Floor;
    
    return mTiles[tileY][tileX].type;
}
