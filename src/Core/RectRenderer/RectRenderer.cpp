#include "RectRenderer.hpp"
#include "../RenderUtils.hpp"
#include <iostream>

RectRenderer::RectRenderer()
    : VAO(0), VBO(0), EBO(0), mWindowWidth(800.0f), mWindowHeight(600.0f)
{
}

RectRenderer::~RectRenderer()
{
    Shutdown();
}

bool RectRenderer::Initialize(float windowWidth, float windowHeight)
{
    mWindowWidth = windowWidth;
    mWindowHeight = windowHeight;
    
    if (!InitializeShaders())
    {
        std::cerr << "Failed to initialize rectangle shaders" << std::endl;
        return false;
    }

    SetupQuadGeometry();
    return true;
}

bool RectRenderer::InitializeShaders()
{
    rectShader = std::make_unique<ShaderProgram>();
    if (!rectShader->CreateFromFiles("shaders/rect.vert", "shaders/rect.frag"))
    {
        std::cerr << "Failed to load rect shaders" << std::endl;
        return false;
    }
    return true;
}

void RectRenderer::SetupQuadGeometry()
{
    // Simple quad vertices (position only)
    // (0,0) is top-left to match text rendering
    float vertices[] = {
        // positions
        0.0f, 0.0f,  // top left
        1.0f, 0.0f,  // top right
        1.0f, 1.0f,  // bottom right
        0.0f, 1.0f   // bottom left
    };

    unsigned int indices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void RectRenderer::RenderRect(float x, float y, float width, float height, const Vector3& color, float alpha)
{
    if (!rectShader)
        return;

    // Enable blending for transparency
    RenderUtils::EnableBlending();

    rectShader->Use();

    // Create transformation matrix
    Matrix4 model = Matrix4::CreateScale(width, height, 1.0f) *
                   Matrix4::CreateTranslation(Vector3(x, y, 0.0f));

    // Create projection matrix for screen coordinates
    Matrix4 projection = RenderUtils::CreateTextProjection(mWindowWidth, mWindowHeight);

    rectShader->SetUniformMatrix4fv("uModel", model.GetAsFloatPtr());
    rectShader->SetUniformMatrix4fv("uProjection", projection.GetAsFloatPtr());
    rectShader->SetUniform3f("uColor", color.x, color.y, color.z);
    rectShader->SetUniform1f("uAlpha", alpha);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    RenderUtils::DisableBlending();
}

void RectRenderer::RenderRectOutline(float x, float y, float width, float height, const Vector3& color, float alpha, float thickness)
{
    // Top
    RenderRect(x, y, width, thickness, color, alpha);
    // Bottom
    RenderRect(x, y + height - thickness, width, thickness, color, alpha);
    // Left
    RenderRect(x, y, thickness, height, color, alpha);
    // Right
    RenderRect(x + width - thickness, y, thickness, height, color, alpha);
}

void RectRenderer::Shutdown()
{
    if (VAO != 0)
    {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO != 0)
    {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (EBO != 0)
    {
        glDeleteBuffers(1, &EBO);
        EBO = 0;
    }
    
    rectShader.reset();
}
