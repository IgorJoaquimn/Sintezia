#include "TileMap.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include <random>

TileMap::TileMap(int width, int height, int tileSize)
    : mWidth(width)
    , mHeight(height)
    , mTileSize(tileSize)
{
    GenerateMap();
}

TileMap::~TileMap()
{
}

Tile TileMap::CreateTile(TileType type)
{
    Tile tile;
    tile.type = type;
    
    switch (type)
    {
        case TileType::Grass:
            tile.emoji = "🟩";
            tile.walkable = true;
            break;
        case TileType::Water:
            tile.emoji = "🟦";
            tile.walkable = false;
            break;
        case TileType::Sand:
            tile.emoji = "🟨";
            tile.walkable = true;
            break;
        case TileType::Tree:
            tile.emoji = "🌲";
            tile.walkable = false;
            break;
        case TileType::Rock:
            tile.emoji = "🪨";
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
            int rand = dis(gen);
            
            // Create varied terrain
            if (rand < 60)
            {
                mTiles[y][x] = CreateTile(TileType::Grass);
            }
            else if (rand < 75)
            {
                mTiles[y][x] = CreateTile(TileType::Sand);
            }
            else if (rand < 85)
            {
                mTiles[y][x] = CreateTile(TileType::Water);
            }
            else if (rand < 95)
            {
                mTiles[y][x] = CreateTile(TileType::Tree);
            }
            else
            {
                mTiles[y][x] = CreateTile(TileType::Rock);
            }
        }
    }
}

void TileMap::Draw(TextRenderer* textRenderer)
{
    if (!textRenderer) return;
    
    for (int y = 0; y < mHeight; y++)
    {
        for (int x = 0; x < mWidth; x++)
        {
            float xPos = x * mTileSize + mTileSize / 4.0f;
            float yPos = y * mTileSize + mTileSize / 4.0f;
            
            textRenderer->RenderText(
                mTiles[y][x].emoji,
                xPos,
                yPos,
                0.8f
            );
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
        return TileType::Grass;
    
    return mTiles[tileY][tileX].type;
}
