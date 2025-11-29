#include "SpriteRenderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

SpriteRenderer::SpriteRenderer()
    : mShader(nullptr)
    , mVAO(0)
    , mVBO(0)
    , mWindowWidth(800.0f)
    , mWindowHeight(600.0f)
    , mCameraPos(Vector2::Zero)
{
}

SpriteRenderer::~SpriteRenderer()
{
    Shutdown();
}

bool SpriteRenderer::Initialize(float windowWidth, float windowHeight)
{
    mWindowWidth = windowWidth;
    mWindowHeight = windowHeight;
    
    if (!InitializeShaders())
    {
        std::cerr << "Failed to initialize sprite shaders!" << std::endl;
        return false;
    }
    
    SetupRenderData();
    
    // std::cout << "SpriteRenderer initialized successfully" << std::endl;
    return true;
}

void SpriteRenderer::Shutdown()
{
    if (mVAO)
    {
        glDeleteVertexArrays(1, &mVAO);
        mVAO = 0;
    }
    if (mVBO)
    {
        glDeleteBuffers(1, &mVBO);
        mVBO = 0;
    }
}

bool SpriteRenderer::InitializeShaders()
{
    mShader = std::make_unique<ShaderProgram>();
    if (!mShader->CreateFromFiles("shaders/vertex.vert", "shaders/fragment.frag"))
    {
        std::cerr << "Failed to load sprite shaders!" << std::endl;
        return false;
    }
    
    return true;
}

void SpriteRenderer::SetupRenderData()
{
    // Configure VAO/VBO
    GLfloat vertices[] = {
        // pos      // tex
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };
    
    glGenVertexArrays(1, &mVAO);
    glGenBuffers(1, &mVBO);
    
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindVertexArray(mVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SpriteRenderer::DrawSprite(Texture* texture, const Vector2& position, const Vector2& size,
                                 float rotation, const Vector3& color)
{
    // Full texture (0,0) to (1,1)
    DrawSprite(texture, position, size, Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), rotation, color, false, false);
}

void SpriteRenderer::DrawSprite(Texture* texture, const Vector2& position, const Vector2& size,
                                 const Vector2& srcPos, const Vector2& srcSize,
                                 float rotation, const Vector3& color,
                                 bool flipHorizontal, bool flipVertical)
{
    // Enable blending for sprite transparency
    glEnable(GL_BLEND);
    // Use separate blend function for alpha to avoid transparency issues when rendering to FBO
    // This ensures that drawing a transparent pixel doesn't reduce the alpha of the destination
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    mShader->Use();
    
    // Prepare transformations
    glm::mat4 model = glm::mat4(1.0f);
    
    // Apply camera translation (inverse of camera position)
    // We subtract camera position to move the world relative to the camera
    Vector2 drawPos = position - mCameraPos;

    // First translate (transformations are: scale happens first, then rotation, then final translation)
    model = glm::translate(model, glm::vec3(drawPos.x, drawPos.y, 0.0f));
    
    // Move origin of rotation to center of quad
    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
    
    // Then rotate
    model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Move origin back
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
    
    // Last scale
    model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));
    
    // Create projection matrix (orthographic)
    glm::mat4 projection = glm::ortho(0.0f, mWindowWidth, mWindowHeight, 0.0f, -1.0f, 1.0f);
    
    mShader->SetUniformMatrix4fv("uWorldTransform", &model[0][0]);
    mShader->SetUniformMatrix4fv("uProjection", &projection[0][0]);
    mShader->SetUniform3f("spriteColor", color.x, color.y, color.z);
    
    // Set texture coordinates for sprite sheet with flipping support
    Vector2 finalSrcPos = srcPos;
    Vector2 finalSrcSize = srcSize;
    
    if (flipHorizontal)
    {
        finalSrcPos.x = srcPos.x + srcSize.x;
        finalSrcSize.x = -srcSize.x;
    }
    
    if (flipVertical)
    {
        finalSrcPos.y = srcPos.y + srcSize.y;
        finalSrcSize.y = -srcSize.y;
    }
    
    mShader->SetUniform2f("texOffset", finalSrcPos.x, finalSrcPos.y);
    mShader->SetUniform2f("texScale", finalSrcSize.x, finalSrcSize.y);
    
    glActiveTexture(GL_TEXTURE0);
    texture->SetActive();
    mShader->SetUniform1i("image", 0);
    
    glBindVertexArray(mVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
