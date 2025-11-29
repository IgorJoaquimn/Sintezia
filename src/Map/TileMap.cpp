#include "TileMap.hpp"
#include "TiledParser.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <SDL.h>
#include <algorithm>
#include <cctype>
#include <glm/glm.hpp>
using json = nlohmann::json;

TileMap::TileMap(int width, int height, int tileSize)
    : mWidth(width)
    , mHeight(height)
    , mTileSize(tileSize)
{
    // Load fallback textures only
    GenerateMap();
}

TileMap::~TileMap()
{
    if (mMapFBO != 0)
    {
        glDeleteFramebuffers(1, &mMapFBO);
        mMapFBO = 0;
    }
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
        
        // Update TileMap dimensions to match loaded map
        mWidth = mMapData->mapWidth;
        mHeight = mMapData->mapHeight;
        
        // Resize mTiles to avoid crashes in IsWalkable
        mTiles.resize(mHeight);
        for (auto& row : mTiles) {
            row.resize(mWidth);
            // Initialize with default
            for (auto& tile : row) {
                tile = CreateTile(TileType::Floor);
            }
        }
        
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
        
        // std::cout << "Successfully loaded tilemap: " << jsonPath << std::endl;
        // std::cout << "  Map size: " << mMapData->mapWidth << "x" << mMapData->mapHeight << std::endl;
        // std::cout << "  Tilesets: " << mMapData->tilesets.size() << std::endl;
        // std::cout << "  Layers: " << mMapData->layers.size() << std::endl;
        
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
        if (!mCachedMapTexture)
        {
            CacheMap(spriteRenderer);
        }

        if (mCachedMapTexture)
        {
            // Draw the cached map texture
            // Note: We need to flip vertically because rendering to FBO results in inverted Y axis
            // relative to our top-left origin coordinate system when drawn as a texture
            spriteRenderer->DrawSprite(
                mCachedMapTexture.get(),
                Vector2(0, 0),
                Vector2(mCachedMapTexture->GetWidth(), mCachedMapTexture->GetHeight()),
                Vector2(0.0f, 0.0f),
                Vector2(1.0f, 1.0f),
                0.0f,
                Vector3(1.0f, 1.0f, 1.0f),
                false,
                true // Flip vertical
            );
        }
        return;
    }
    
    // Otherwise, do not draw procedural tilemap (no textures)
}

void TileMap::CacheMap(SpriteRenderer* spriteRenderer)
{
    if (!mMapData) return;

    int width = mMapData->mapWidth * mTileSize;
    int height = mMapData->mapHeight * mTileSize;
    
    // Create texture if needed
    if (!mCachedMapTexture)
    {
        mCachedMapTexture = std::make_unique<Texture>();
        mCachedMapTexture->CreateForRendering(width, height, GL_RGBA);
    }
    
    // Create FBO if needed
    if (mMapFBO == 0)
    {
        glGenFramebuffers(1, &mMapFBO);
    }
    
    // Save current state
    GLint prevFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
    
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    
    float prevWidth = spriteRenderer->GetWindowWidth();
    float prevHeight = spriteRenderer->GetWindowHeight();
    
    // Setup FBO
    glBindFramebuffer(GL_FRAMEBUFFER, mMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mCachedMapTexture->GetTextureID(), 0);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
        return;
    }
    
    // Setup rendering
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Transparent background
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Set projection to match map size
    spriteRenderer->SetProjection(width, height);
    
    static bool debugPrinted = false;
    if (!debugPrinted)
    {
        // std::cout << "Caching Tiled map to texture (" << width << "x" << height << ")" << std::endl;
        debugPrinted = true;
    }
    
    // Draw each layer
    for (const auto& layer : mMapData->layers)
    {
        if (layer.data.empty()) continue;
        
        // Skip special layers
        if (layer.name == "collision" || layer.name.find("gerador_") == 0)
            continue;
        
        // Draw each tile in the layer
        for (int y = 0; y < layer.height; y++)
        {
            for (int x = 0; x < layer.width; x++)
            {
                int index = y * layer.width + x;
                if (index >= layer.data.size()) continue;
                
                int gid = layer.data[index];
                if (gid == 0) continue; // 0 = empty tile
                
                // Extract flip flags from GID (Tiled format)
                const unsigned FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
                const unsigned FLIPPED_VERTICALLY_FLAG   = 0x40000000;
                const unsigned FLIPPED_DIAGONALLY_FLAG   = 0x20000000;
                
                bool flippedHorizontally = (gid & FLIPPED_HORIZONTALLY_FLAG);
                bool flippedVertically = (gid & FLIPPED_VERTICALLY_FLAG);
                bool flippedDiagonally = (gid & FLIPPED_DIAGONALLY_FLAG);
                
                // Clear the flags to get the actual tile ID
                gid &= ~(FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG);
                
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
                // In Tiled: (x, y) is the grid cell position, with (0,0) at top-left
                float destX = x * mTileSize;
                float destY = y * mTileSize;
                
                // Calculate display size based on the tileset's tile size
                float scaleFactorX = static_cast<float>(tileset->tileWidth) / static_cast<float>(mMapData->tileWidth);
                float scaleFactorY = static_cast<float>(tileset->tileHeight) / static_cast<float>(mMapData->tileHeight);
                
                // Display size in screen pixels
                float displayWidth = mTileSize * scaleFactorX;
                float displayHeight = mTileSize * scaleFactorY;
                
                // Offset scale for converting Tiled pixels to display pixels
                float offsetScale = mTileSize / 16.0f;
                
                // Apply offsets from Tiled editor
                destX += tileset->offsetX * offsetScale;
                destY -= tileset->offsetY * offsetScale;
                
                // Handle Tiled's flip flags
                float rotation = 0.0f;
                bool flipH = flippedHorizontally;
                bool flipV = flippedVertically;
                
                if (flippedDiagonally)
                {
                    rotation = glm::radians(90.0f);
                    if (flippedHorizontally && flippedVertically)
                    {
                        rotation = glm::radians(270.0f);
                        flipH = false;
                    }
                    else if (flippedVertically)
                    {
                        rotation = glm::radians(270.0f);
                        flipH = false;
                        flipV = false;
                    }
                    else if (flippedHorizontally)
                    {
                        flipH = false;
                        flipV = false;
                    }
                }
                else if (flippedHorizontally && flippedVertically)
                {
                    rotation = glm::radians(180.0f);
                    flipH = false;
                    flipV = false;
                }
                
                // Draw the tile
                spriteRenderer->DrawSprite(
                    tileset->texture.get(),
                    Vector2(destX, destY),
                    Vector2(displayWidth, displayHeight),
                    Vector2(normalizedSrcX, normalizedSrcY),
                    Vector2(normalizedWidth, normalizedHeight),
                    rotation,
                    Vector3(1.0f, 1.0f, 1.0f),
                    flipH,
                    flipV
                );
            }
        }
    }
    
    // Restore state
    spriteRenderer->SetProjection(prevWidth, prevHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
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


bool TileMap::CheckCollision(const Vector2& position, float radius) const
{

    // If we have Tiled map data, check against collision tiles
    if (mMapData && !mMapData->layers.empty())
    {
        // Check all four corners of the player's bounding box
        Vector2 corners[4] = {
            Vector2(position.x - radius, position.y - radius), // Top-left
            Vector2(position.x + radius, position.y - radius), // Top-right
            Vector2(position.x - radius, position.y + radius), // Bottom-left
            Vector2(position.x + radius, position.y + radius)  // Bottom-right
        };
        

        for (const auto& corner : corners)
        {
            int tileX = static_cast<int>(corner.x) / mTileSize;
            int tileY = static_cast<int>(corner.y) / mTileSize;
            
            // Out of bounds = collision
            if (tileX < 0 || tileX >= mMapData->mapWidth || tileY < 0 || tileY >= mMapData->mapHeight)
                return true;
            
            // Check each layer for collision tiles
            for (const auto& layer : mMapData->layers)
            {
                if (layer.data.empty()) continue;
                
                int index = tileY * layer.width + tileX;
                if (index < 0 || index >= layer.data.size()) continue;
                
                int gid = layer.data[index];
                if (gid == 0) continue; // Empty tile
                
                // Special check for collision layer
                if (layer.name == "collision")
                {
                    return true;
                }
            }
        }
        
        return false; // No collision
    }
    
    // Fallback to simple tile checking
    return !IsWalkable(position);
}
