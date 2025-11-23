#include "MapHelper.hpp"
#include <algorithm>

// Find which tileset a GID belongs to
int FindTilesetIndex(const MapData* map, int gid)
{
    if (!map || map->tilesets.empty()) return -1;
    
    for (int i = static_cast<int>(map->tilesets.size()) - 1; i >= 0; i--)
    {
        if (gid >= map->tilesets[i].firstGid)
        {
            return i;
        }
    }
    return 0;
}

// Extract flip/rotation info from GID
TileRenderInfo GetTileFlipInfoFromGID(uint32_t gidWithFlags)
{
    const uint32_t FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
    const uint32_t FLIPPED_VERTICALLY_FLAG   = 0x40000000;
    const uint32_t FLIPPED_DIAGONALLY_FLAG   = 0x20000000;
    
    TileRenderInfo info;
    info.cleanGid = gidWithFlags & ~(FLIPPED_HORIZONTALLY_FLAG | 
                                      FLIPPED_VERTICALLY_FLAG | 
                                      FLIPPED_DIAGONALLY_FLAG);
    info.angleDeg = 0.0;
    info.flip = SDL_FLIP_NONE;
    
    bool flipH = (gidWithFlags & FLIPPED_HORIZONTALLY_FLAG) != 0;
    bool flipV = (gidWithFlags & FLIPPED_VERTICALLY_FLAG) != 0;
    bool flipD = (gidWithFlags & FLIPPED_DIAGONALLY_FLAG) != 0;
    
    if (flipD)
    {
        if (flipH && flipV)
        {
            info.angleDeg = 90.0;
            info.flip = SDL_FLIP_NONE;
        }
        else if (flipH)
        {
            info.angleDeg = 90.0;
            info.flip = SDL_FLIP_HORIZONTAL;
        }
        else if (flipV)
        {
            info.angleDeg = 90.0;
            info.flip = SDL_FLIP_VERTICAL;
        }
        else
        {
            info.angleDeg = 270.0;
            info.flip = SDL_FLIP_HORIZONTAL;
        }
    }
    else
    {
        if (flipH && flipV)
        {
            info.flip = static_cast<SDL_RendererFlip>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
        }
        else if (flipH)
        {
            info.flip = SDL_FLIP_HORIZONTAL;
        }
        else if (flipV)
        {
            info.flip = SDL_FLIP_VERTICAL;
        }
    }
    
    return info;
}

// Get layer index by name
int GetLayerIdx(const MapData& map, const std::string& layerName)
{
    for (size_t i = 0; i < map.layers.size(); i++)
    {
        if (map.layers[i].name == layerName)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}
