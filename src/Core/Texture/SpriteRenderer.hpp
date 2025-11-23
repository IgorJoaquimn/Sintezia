#pragma once
#include <GL/glew.h>
#include <memory>
#include "../../Shader/ShaderProgram.hpp"
#include "Texture.hpp"
#include "../../Math.h"

class SpriteRenderer
{
public:
    SpriteRenderer();
    ~SpriteRenderer();
    
    bool Initialize(float windowWidth, float windowHeight);
    void Shutdown();
    
    // Draw a sprite
    void DrawSprite(Texture* texture, const Vector2& position, const Vector2& size, 
                    float rotation = 0.0f, const Vector3& color = Vector3(1.0f, 1.0f, 1.0f));
    
    // Draw a sprite with source rect (for sprite sheets)
    void DrawSprite(Texture* texture, const Vector2& position, const Vector2& size,
                    const Vector2& srcPos, const Vector2& srcSize,
                    float rotation = 0.0f, const Vector3& color = Vector3(1.0f, 1.0f, 1.0f),
                    bool flipHorizontal = false, bool flipVertical = false);
    
private:
    bool InitializeShaders();
    void SetupRenderData();
    
    std::unique_ptr<ShaderProgram> mShader;
    GLuint mVAO;
    GLuint mVBO;
    float mWindowWidth;
    float mWindowHeight;
};
