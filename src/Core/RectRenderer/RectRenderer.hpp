#pragma once
#include <GL/glew.h>
#include <memory>
#include "../../Shader/ShaderProgram.hpp"
#include "../../MathUtils.h"

class RectRenderer {
public:
    RectRenderer();
    ~RectRenderer();

    bool Initialize(float windowWidth = 800.0f, float windowHeight = 600.0f);
    void RenderRect(float x, float y, float width, float height, const Vector3& color, float alpha = 1.0f);
    void Shutdown();

private:
    std::unique_ptr<ShaderProgram> rectShader;
    GLuint VAO, VBO, EBO;
    
    // Window dimensions for projection
    float mWindowWidth;
    float mWindowHeight;
    
    bool InitializeShaders();
    void SetupQuadGeometry();
};
