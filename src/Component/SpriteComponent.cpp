#include "SpriteComponent.hpp"
#include "../Actor/Actor.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include <SDL.h>

SpriteComponent::SpriteComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mTexture(std::make_unique<Texture>())
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
    if (!mTexture->Load(filepath))
    {
        SDL_Log("Failed to load sprite sheet: %s", filepath.c_str());
        return false;
    }
    return true;
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
    
    // Calculate texture coordinates (assuming 192x320 sprite sheet for now)
    float sheetWidth = 192.0f;
    float sheetHeight = 320.0f;
    
    Vector2 srcPos((mCurrentCol * mSpriteWidth) / sheetWidth, 
                   (mCurrentRow * mSpriteHeight) / sheetHeight);
    Vector2 srcSize(mSpriteWidth / sheetWidth, mSpriteHeight / sheetHeight);
    
    // Draw position (centered on actor position)
    Vector2 pos = mOwner->GetPosition();
    Vector2 drawPos(pos.x - mRenderSize/2, pos.y - mRenderSize/2);
    Vector2 drawSize(mRenderSize, mRenderSize);
    
    // Draw using sprite sheet with source rectangle
    renderer->DrawSprite(mTexture.get(), drawPos, drawSize, srcPos, srcSize, 0.0f, 
                        Vector3(1.0f, 1.0f, 1.0f), mFlipHorizontal, false);
}
