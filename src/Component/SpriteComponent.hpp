#pragma once
#include "Component.hpp"
#include "../Core/Texture/Texture.hpp"
#include "../Math.h"
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
    
private:
    std::unique_ptr<Texture> mTexture;
    int mSpriteWidth;
    int mSpriteHeight;
    int mCurrentRow;
    int mCurrentCol;
    float mRenderSize;
    bool mFlipHorizontal;
};
