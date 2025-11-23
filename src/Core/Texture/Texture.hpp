#pragma once
#include <GL/glew.h>
#include <string>

class Texture
{
public:
    Texture();
    ~Texture();
    
    bool Load(const std::string& fileName);
    void Unload();
    
    void SetActive();
    
    int GetWidth() const { return mWidth; }
    int GetHeight() const { return mHeight; }
    GLuint GetTextureID() const { return mTextureID; }
    
private:
    GLuint mTextureID;
    int mWidth;
    int mHeight;
};
