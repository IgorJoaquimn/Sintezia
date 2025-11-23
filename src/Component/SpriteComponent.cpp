#include "SpriteComponent.hpp"
#include "../Actor/Actor.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include <SDL.h>

SpriteComponent::SpriteComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mTexture(nullptr)
    , mSpriteWidth(32)
    , mSpriteHeight(32)
    , mCurrentRow(0)
    , mCurrentCol(0)
    , mRenderSize(80.0f)
    , mFlipHorizontal(false)
{
}

SpriteComponent::~SpriteComponent()
{
}

bool SpriteComponent::LoadSpriteSheet(const std::string& filepath)
{
    auto texture = std::make_shared<Texture>();
    if (!texture->Load(filepath))
    {
        SDL_Log("Failed to load sprite sheet: %s", filepath.c_str());
        return false;
    }
    mTexture = texture;
    return true;
}

void SpriteComponent::SetTexture(std::shared_ptr<Texture> texture)
{
    mTexture = texture;
}

void SpriteComponent::SetSpriteSize(int width, int height)
{
    mSpriteWidth = width;
    mSpriteHeight = height;
}

void SpriteComponent::SetCurrentFrame(int row, int col)
{
    mCurrentRow = row;
    mCurrentCol = col;
}

void SpriteComponent::Draw(SpriteRenderer* renderer)
{
    if (!renderer || !mTexture) return;
    
    // If sprite size is 0 or texture size, use full texture
    float texW = static_cast<float>(mTexture->GetWidth());
    float texH = static_cast<float>(mTexture->GetHeight());
    
    // Default to full texture if sprite size is not set or matches texture
    float spriteW = (mSpriteWidth > 0) ? static_cast<float>(mSpriteWidth) : texW;
    float spriteH = (mSpriteHeight > 0) ? static_cast<float>(mSpriteHeight) : texH;
    
    // Calculate texture coordinates
    // If using a grid, sheetWidth/Height would be the texture dimensions
    
    Vector2 srcPos((mCurrentCol * spriteW) / texW, 
                   (mCurrentRow * spriteH) / texH);
    Vector2 srcSize(spriteW / texW, spriteH / texH);
    
    // Draw position (centered on actor position)
    Vector2 pos = mOwner->GetPosition();
    Vector2 drawPos(pos.x - mRenderSize/2, pos.y - mRenderSize/2);
    Vector2 drawSize(mRenderSize, mRenderSize);
    
    // Draw using sprite sheet with source rectangle
    renderer->DrawSprite(mTexture.get(), drawPos, drawSize, srcPos, srcSize, 0.0f, 
                        Vector3(1.0f, 1.0f, 1.0f), mFlipHorizontal, false);
}
