#include "Texture.hpp"
#include <SDL_image.h>
#include <iostream>

Texture::Texture()
    : mTextureID(0)
    , mWidth(0)
    , mHeight(0)
{
}

Texture::~Texture()
{
    Unload();
}

bool Texture::Load(const std::string& fileName)
{
    // Load from file
    SDL_Surface* surf = IMG_Load(fileName.c_str());
    if (!surf)
    {
        std::cerr << "Failed to load texture file " << fileName << ": " << SDL_GetError() << std::endl;
        return false;
    }
    
    mWidth = surf->w;
    mHeight = surf->h;
    
    // Determine format
    int format = GL_RGB;
    if (surf->format->BytesPerPixel == 4)
    {
        format = GL_RGBA;
    }
    
    // Generate texture
    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, mWidth, mHeight, 0, format, GL_UNSIGNED_BYTE, surf->pixels);
    
    SDL_FreeSurface(surf);
    
    // Use nearest-neighbor filtering for crisp pixel art (no blur)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
    return true;
}

bool Texture::CreateForRendering(int width, int height, unsigned int format)
{
    mWidth = width;
    mHeight = height;
    
    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, mWidth, mHeight, 0, format, GL_UNSIGNED_BYTE, nullptr);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    return true;
}

void Texture::Unload()
{
    if (mTextureID != 0)
    {
        glDeleteTextures(1, &mTextureID);
        mTextureID = 0;
    }
}

void Texture::SetActive()
{
    glBindTexture(GL_TEXTURE_2D, mTextureID);
}
