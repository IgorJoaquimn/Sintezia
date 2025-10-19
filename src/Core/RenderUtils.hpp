#pragma once
#include "../Math.h"
#include <GL/glew.h>

class RenderUtils {
public:
    // Create orthographic projection matrix for text rendering (0,0 at top-left)
    static Matrix4 CreateTextProjection(float width, float height) {
        float temp[4][4] = {
            { 2.0f/width, 0.0f, 0.0f, 0.0f },
            { 0.0f, -2.0f/height, 0.0f, 0.0f },  // Negative Y to flip coordinates
            { 0.0f, 0.0f, -1.0f, 0.0f },
            { -1.0f, 1.0f, 0.0f, 1.0f }         // Adjusted translation
        };
        return Matrix4(temp);
    }
    
    // Common projection matrix creation (generic)
    static Matrix4 CreateOrthographicProjection(float width, float height) {
        return Matrix4::CreateSimpleViewProj(width, height);
    }
    
    // Common OpenGL state management
    static void EnableBlending() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    
    static void DisableBlending() {
        glDisable(GL_BLEND);
    }
    
    // Common texture binding
    static void BindTexture(GLuint textureID, GLenum textureUnit = GL_TEXTURE0) {
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }
    
    static void UnbindTexture() {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    // Common VAO binding
    static void BindVAO(GLuint vao) {
        glBindVertexArray(vao);
    }
    
    static void UnbindVAO() {
        glBindVertexArray(0);
    }
    
    // Clear screen with color
    static void ClearScreen(float r = 0.1f, float g = 0.1f, float b = 0.1f, float a = 1.0f) {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT);
    }
};