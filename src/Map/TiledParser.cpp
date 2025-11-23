#include "TiledParser.hpp"
#include <fstream>
#include <iostream>

std::string TiledParser::ExtractAttribute(const std::string& line, const std::string& attrName)
{
    std::string searchStr = attrName + "=\"";
    size_t pos = line.find(searchStr);
    if (pos == std::string::npos)
        return "";
    
    pos += searchStr.length();
    size_t endPos = line.find("\"", pos);
    if (endPos == std::string::npos)
        return "";
    
    return line.substr(pos, endPos - pos);
}

bool TiledParser::ParseTSX(const std::string& tsxPath, TilesetInfo& tileset)
{
    std::ifstream file(tsxPath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open TSX file: " << tsxPath << std::endl;
        return false;
    }
    
    // Initialize default values
    tileset.spacing = 0;
    tileset.margin = 0;
    
    std::string line;
    while (std::getline(file, line))
    {
        // Parse <tileset> tag
        if (line.find("<tileset") != std::string::npos)
        {
            std::string val;
            
            val = ExtractAttribute(line, "tilewidth");
            if (!val.empty()) tileset.tileWidth = std::stoi(val);
            
            val = ExtractAttribute(line, "tileheight");
            if (!val.empty()) tileset.tileHeight = std::stoi(val);
            
            val = ExtractAttribute(line, "tilecount");
            if (!val.empty()) tileset.tileCount = std::stoi(val);
            
            val = ExtractAttribute(line, "columns");
            if (!val.empty()) tileset.columns = std::stoi(val);
            
            val = ExtractAttribute(line, "spacing");
            if (!val.empty()) tileset.spacing = std::stoi(val);
            
            val = ExtractAttribute(line, "margin");
            if (!val.empty()) tileset.margin = std::stoi(val);
        }
        
        // Parse <tileoffset> tag
        if (line.find("<tileoffset") != std::string::npos)
        {
            std::string val;
            
            val = ExtractAttribute(line, "x");
            if (!val.empty()) tileset.offsetX = std::stoi(val);
            
            val = ExtractAttribute(line, "y");
            if (!val.empty()) tileset.offsetY = std::stoi(val);
        }
        
        // Parse <image> tag
        if (line.find("<image") != std::string::npos)
        {
            std::string source = ExtractAttribute(line, "source");
            if (!source.empty())
            {
                // Resolve path relative to TSX file location
                std::string basePath = tsxPath.substr(0, tsxPath.find_last_of("/\\") + 1);
                std::string fullPath = basePath + source;
                
                // Clean up path (remove ../ by going up directories)
                while (fullPath.find("../") != std::string::npos)
                {
                    size_t pos = fullPath.find("../");
                    if (pos == 0) break; // Can't go up from root
                    
                    // Find the directory before ../
                    size_t slashBefore = fullPath.rfind("/", pos - 2);
                    if (slashBefore == std::string::npos) break;
                    
                    // Remove the directory and the ../
                    fullPath = fullPath.substr(0, slashBefore + 1) + fullPath.substr(pos + 3);
                }
                
                tileset.imagePath = fullPath;
            }
        }
    }
    
    // Calculate rows
    if (tileset.tileCount > 0 && tileset.columns > 0)
    {
        tileset.rows = (tileset.tileCount + tileset.columns - 1) / tileset.columns;
    }
    
    // Initialize collision data (default: no collision)
    tileset.tileCollisions.resize(tileset.tileCount, false);
    
    // Re-open file to parse tile properties
    file.clear();
    file.seekg(0);
    
    int currentTileId = -1;
    while (std::getline(file, line))
    {
        // Check for <tile id="X">
        if (line.find("<tile") != std::string::npos && line.find("id=") != std::string::npos)
        {
            std::string idStr = ExtractAttribute(line, "id");
            if (!idStr.empty())
            {
                currentTileId = std::stoi(idStr);
            }
        }
        
        // Check for collision property
        if (currentTileId >= 0 && currentTileId < tileset.tileCount)
        {
            if (line.find("<property") != std::string::npos && 
                line.find("name=\"collision\"") != std::string::npos)
            {
                std::string value = ExtractAttribute(line, "value");
                if (value == "true")
                {
                    tileset.tileCollisions[currentTileId] = true;
                }
            }
        }
        
        // Reset tile ID when closing tag
        if (line.find("</tile>") != std::string::npos)
        {
            currentTileId = -1;
        }
    }
    
    file.close();
    
    // Load the texture
    tileset.texture = std::make_unique<Texture>();
    if (!tileset.texture->Load(tileset.imagePath))
    {
        std::cerr << "Failed to load tileset image from TSX: " << tileset.imagePath << std::endl;
        return false;
    }
    
    std::cout << "Loaded TSX tileset: " << tsxPath << std::endl;
    std::cout << "  Image: " << tileset.imagePath << std::endl;
    std::cout << "  Tile size: " << tileset.tileWidth << "x" << tileset.tileHeight << std::endl;
    std::cout << "  Tiles: " << tileset.tileCount << " (" << tileset.columns << " columns)" << std::endl;
    
    // Count collision tiles
    int collisionCount = 0;
    for (bool hasCollision : tileset.tileCollisions)
    {
        if (hasCollision) collisionCount++;
    }
    if (collisionCount > 0)
    {
        std::cout << "  Collision tiles: " << collisionCount << std::endl;
    }
    
    return true;
}
