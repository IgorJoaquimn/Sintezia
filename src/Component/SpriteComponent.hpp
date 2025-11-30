#pragma once
#include "Component.hpp"
#include "../Core/Texture/Texture.hpp"
#include "../MathUtils.h"
#include <memory>
#include <string>

// Component that handles sprite rendering
class SpriteComponent : public Component
{
public:
    SpriteComponent(class Actor* owner, int updateOrder = 200);
    ~SpriteComponent();
    
    // Load sprite sheet
    bool LoadSpriteSheet(const std::string& filepath);
    
    // Rendering
    void Draw(class SpriteRenderer* renderer);
    
    // Sprite sheet configuration
    void SetSpriteSize(int width, int height);
    void SetCurrentFrame(int row, int col);
    void SetFlipHorizontal(bool flip) { mFlipHorizontal = flip; }
    void SetRenderSize(float size) { mRenderSize = size; }
    
    // Getters
    Texture* GetTexture() const { return mTexture.get(); }
    
    // Set texture directly
    void SetTexture(std::shared_ptr<Texture> texture);

    // Set color tint (RGB)
    void SetColor(const Vector3& color) { mColor = color; }
    const Vector3& GetColor() const { return mColor; }

private:
    std::shared_ptr<Texture> mTexture;
    int mSpriteWidth;
    int mSpriteHeight;
    int mCurrentRow;
    int mCurrentCol;
    float mRenderSize;
    bool mFlipHorizontal;
    Vector3 mColor;
};
