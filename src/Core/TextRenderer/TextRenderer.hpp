#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <GL/glew.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

struct GlyphInfo {
    GLuint textureID;
    int width, height;
    int bearingX, bearingY;
    int advance;
    bool isColor; // true for emoji, false for regular text
};

struct AtlasRegion {
    int x, y, width, height;
};

class TextRenderer {
public:
    TextRenderer();
    ~TextRenderer();

    bool Initialize();
    void RenderText(const std::string& text, float x, float y, float scale = 1.0f);

private:
    FT_Library ftLibrary;
    FT_Face textFace;     // Regular text font
    FT_Face emojiFace;    // Emoji font
    
    // Glyph cache for both text and emojis
    std::unordered_map<uint32_t, GlyphInfo> glyphCache;
    
    // Atlas for glyph textures
    GLuint atlasTexture;
    int atlasWidth, atlasHeight;
    int currentX, currentY, rowHeight;
    
    // OpenGL rendering resources
    GLuint VAO, VBO;
    GLuint shaderProgram;
    
    bool LoadFonts();
    bool CreateShaders();
    GlyphInfo LoadGlyph(uint32_t codepoint, FT_Face face, bool isEmoji);
    bool CreateAtlas();
    AtlasRegion AllocateAtlasRegion(int width, int height);
    uint32_t GetNextCodepoint(const std::string& text, size_t& pos);
};