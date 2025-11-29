#include "Texture.hpp"
#include <SDL_image.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>

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
    // Suppress libpng warnings by redirecting stderr
    int stderr_backup = dup(STDERR_FILENO);
    int null_fd = open("/dev/null", O_WRONLY);
    if (stderr_backup != -1 && null_fd != -1) {
        dup2(null_fd, STDERR_FILENO);
    }

    SDL_Surface* surf = IMG_Load(fileName.c_str());

    // Restore stderr
    if (stderr_backup != -1 && null_fd != -1) {
        fflush(stderr);
        dup2(stderr_backup, STDERR_FILENO);
        close(stderr_backup);
        close(null_fd);
    }

    if (!surf)
    {
        std::cerr << "Failed to load texture file " << fileName << ": " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Convert indexed/palette images to RGBA to avoid OpenGL errors
    SDL_Surface* formattedSurf = surf;
    bool converted = false;
    
    if (surf->format->BytesPerPixel != 3 && surf->format->BytesPerPixel != 4)
    {
        // Convert to ABGR8888 (which is R,G,B,A in memory on little endian, matching GL_RGBA)
        formattedSurf = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ABGR8888, 0);
        if (formattedSurf)
        {
            converted = true;
        }
        else
        {
            std::cerr << "Failed to convert surface format for " << fileName << ": " << SDL_GetError() << std::endl;
            // Fallback to original surface, though it will likely fail rendering
        }
    }

    mWidth = formattedSurf->w;
    mHeight = formattedSurf->h;
    
    // Determine format
    int format = GL_RGB;
    if (formattedSurf->format->BytesPerPixel == 4)
    {
        format = GL_RGBA;
    }
    
    // Generate texture
    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, mWidth, mHeight, 0, format, GL_UNSIGNED_BYTE, formattedSurf->pixels);
    
    if (converted)
    {
        SDL_FreeSurface(formattedSurf);
    }
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
