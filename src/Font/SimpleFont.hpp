#pragma once
#include <GL/glew.h>
#include <string>
#include <unordered_map>
#include <vector>

struct Character {
    GLuint textureID;   // ID handle of the glyph texture
    int width, height;  // Size of glyph
    int bearingX, bearingY; // Offset from baseline to left/top of glyph
    int advance;        // Offset to advance to next glyph
    bool isColor;       // Whether this is a color emoji texture
};

enum class FontType {
    REGULAR,
    COLOR_EMOJI
};

class SimpleFont {
public:
    SimpleFont();
    ~SimpleFont();
    
    bool LoadFont(const std::string& fontPath, int fontSize);
    bool LoadFallbackFont(const std::string& fontPath, int fontSize); // For regular text fallback
    void RenderText(const std::string& text, float x, float y, float scale = 1.0f);
    
private:
    std::unordered_map<char, Character> characters;
    std::unordered_map<uint32_t, Character> unicodeCharacters; // For emoji and Unicode
    GLuint VAO, VBO;
    GLuint shaderProgram;
    
    bool CreateShaders();
    void LoadCharacters();
    void LoadUnicodeCharacter(uint32_t codepoint);
    std::vector<uint32_t> UTF8ToCodepoints(const std::string& utf8);
    bool IsColorEmojiFont(void* face);
    void SetupColorFont(void* face, int fontSize);
    void SetupRegularFont(void* face, int fontSize);
    
    // FreeType library and faces
    void* library; // FT_Library but as void* to avoid including freetype in header
    void* face;    // FT_Face for main font (could be emoji or regular)
    void* fallbackFace; // FT_Face for fallback font
    FontType mainFontType;
};