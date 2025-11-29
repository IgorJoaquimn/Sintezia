#pragma once
#include <GL/glew.h>
#include <memory>
#include "../../Shader/ShaderProgram.hpp"
#include "Texture.hpp"
#include "../../MathUtils.h"

class SpriteRenderer
{
public:
    SpriteRenderer();
    ~SpriteRenderer();
    
    bool Initialize(float windowWidth, float windowHeight);
    void Shutdown();

    void SetProjection(float width, float height) { mWindowWidth = width; mWindowHeight = height; }
    float GetWindowWidth() const { return mWindowWidth; }
    float GetWindowHeight() const { return mWindowHeight; }
    
    // Draw a sprite
    void DrawSprite(Texture* texture, const Vector2& position, const Vector2& size, 
                    float rotation = 0.0f, const Vector3& color = Vector3(1.0f, 1.0f, 1.0f));
    
    // Draw a sprite with source rect (for sprite sheets)
    void DrawSprite(Texture* texture, const Vector2& position, const Vector2& size,
                    const Vector2& srcPos, const Vector2& srcSize,
                    float rotation = 0.0f, const Vector3& color = Vector3(1.0f, 1.0f, 1.0f),
                    bool flipHorizontal = false, bool flipVertical = false);
    
    // Set camera position for rendering
    void SetCameraPosition(const Vector2& pos) { mCameraPos = pos; }
    const Vector2& GetCameraPosition() const { return mCameraPos; }

private:
    bool InitializeShaders();
    void SetupRenderData();
    
    std::unique_ptr<ShaderProgram> mShader;
    GLuint mVAO;
    GLuint mVBO;
    float mWindowWidth;
    float mWindowHeight;
    Vector2 mCameraPos;
};
