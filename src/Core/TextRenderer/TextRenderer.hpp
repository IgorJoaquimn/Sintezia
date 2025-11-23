#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <GL/glew.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include "../../Font/FontManager.hpp"
#include "../../Shader/ShaderProgram.hpp"
#include "../../MathUtils.h"
#include "../RenderUtils.hpp"

struct GlyphInfo {
    GLuint textureID;
    int width, height;
    int bearingX, bearingY;
    int advance;
    bool isColor; // true for emoji, false for regular text
};

class TextRenderer {
public:
    TextRenderer();
    ~TextRenderer();

    bool Initialize(float windowWidth = 800.0f, float windowHeight = 600.0f);
    void RenderText(const std::string& text, float x, float y, float scale = 1.0f);
    void SetTextColor(float r, float g, float b) { mTextColor = Vector3(r, g, b); }
    
    // Calculate text dimensions
    Vector2 MeasureText(const std::string& text, float scale = 1.0f) const;
    float GetTextWidth(const std::string& text, float scale = 1.0f) const;
    float GetTextHeight(const std::string& text, float scale = 1.0f) const;

private:
    std::unique_ptr<FontManager> fontManager;
    std::unique_ptr<ShaderProgram> textShader;
    Vector3 mTextColor;
    
    // Window dimensions for projection
    float mWindowWidth;
    float mWindowHeight;
    
    // Glyph cache for both text and emojis
    std::unordered_map<uint32_t, GlyphInfo> glyphCache;
    
    // OpenGL rendering resources
    GLuint VAO, VBO;
    
    bool InitializeShaders();
    GlyphInfo LoadGlyph(uint32_t codepoint, FT_Face face, bool isEmoji);
    uint32_t GetNextCodepoint(const std::string& text, size_t& pos) const;
    bool IsEmojiCodepoint(uint32_t codepoint) const;
};