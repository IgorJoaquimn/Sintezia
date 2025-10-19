#pragma once
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <unordered_map>
#include <memory>

struct SDLCharacter {
    GLuint textureID;
    int width, height;
    int bearingX, bearingY;
    int advance;
};

class SDLFont {
public:
    SDLFont();
    ~SDLFont();
    
    bool Initialize();
    bool LoadFont(const std::string& fontPath, int fontSize);
    void RenderText(const std::string& text, float x, float y, float scale = 1.0f);
    
private:
    TTF_Font* font;
    TTF_Font* fallbackFont;
    std::unordered_map<std::string, SDLCharacter> textCache;
    GLuint VAO, VBO;
    GLuint shaderProgram;
    
    bool CreateShaders();
    SDL_Surface* RenderTextSurface(const std::string& text);
    GLuint CreateTextureFromSurface(SDL_Surface* surface);
    void CacheText(const std::string& text);
};