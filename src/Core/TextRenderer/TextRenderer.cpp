#include "TextRenderer.hpp"
#include <iostream>
#include <cstring>

TextRenderer::TextRenderer()
    : fontManager(std::make_unique<FontManager>()),
      textShader(std::make_unique<ShaderProgram>()),
      mTextColor(1.0f, 1.0f, 1.0f),
      VAO(0), VBO(0) {
}

TextRenderer::~TextRenderer() {
    // Clean up glyph textures
    for (auto& pair : glyphCache) {
        if (pair.second.textureID) {
            glDeleteTextures(1, &pair.second.textureID);
        }
    }

    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
}

bool TextRenderer::Initialize() {
    if (!fontManager->Initialize()) {
        std::cerr << "ERROR: Could not initialize FontManager!" << std::endl;
        return false;
    }

    if (!InitializeShaders()) {
        std::cerr << "ERROR: Could not create shaders!" << std::endl;
        return false;
    }

    // Create VAO/VBO for text rendering
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    std::cout << "TextRenderer initialized successfully" << std::endl;
    return true;
}

bool TextRenderer::InitializeShaders() {
    if (!textShader->CreateFromFiles("shaders/text.vert", "shaders/text.frag")) {
        std::cout << "ERROR: Failed to load text rendering shaders!" << std::endl;
        return false;
    }
    
    std::cout << "Text rendering shaders loaded successfully" << std::endl;
    return true;
}

bool TextRenderer::IsEmojiCodepoint(uint32_t codepoint) const {
    return (codepoint >= 0x1F300 && codepoint <= 0x1F9FF) ||  // Misc symbols and pictographs
           (codepoint >= 0x1F600 && codepoint <= 0x1F64F) ||  // Emoticons
           (codepoint >= 0x1F680 && codepoint <= 0x1F6FF) ||  // Transport and map symbols
           (codepoint >= 0x2600 && codepoint <= 0x26FF) ||   // Misc symbols
           (codepoint >= 0x2700 && codepoint <= 0x27BF) ||   // Dingbats
           (codepoint >= 0x1F1E0 && codepoint <= 0x1F1FF);   // Regional indicator symbols
}

uint32_t TextRenderer::GetNextCodepoint(const std::string& text, size_t& pos) const {
    if (pos >= text.length()) return 0;

    uint8_t c = static_cast<uint8_t>(text[pos++]);
    uint32_t codepoint = c;

    // UTF-8 decoding - robust checks
    if ((c & 0x80) == 0) {
        return codepoint;
    } else if ((c & 0xE0) == 0xC0) {
        if (pos < text.length()) {
            uint8_t c2 = static_cast<uint8_t>(text[pos++]);
            codepoint = ((c & 0x1F) << 6) | (c2 & 0x3F);
        }
    } else if ((c & 0xF0) == 0xE0) {
        if (pos + 1 < text.length()) {
            uint8_t c2 = static_cast<uint8_t>(text[pos++]);
            uint8_t c3 = static_cast<uint8_t>(text[pos++]);
            codepoint = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
        }
    } else if ((c & 0xF8) == 0xF0) {
        if (pos + 2 < text.length()) {
            uint8_t c2 = static_cast<uint8_t>(text[pos++]);
            uint8_t c3 = static_cast<uint8_t>(text[pos++]);
            uint8_t c4 = static_cast<uint8_t>(text[pos++]);
            codepoint = ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
        }
    }

    return codepoint;
}

GlyphInfo TextRenderer::LoadGlyph(uint32_t codepoint, FT_Face face, bool isEmoji) {
    GlyphInfo glyph = {};
    glyph.textureID = 0;

    FT_Int32 loadFlags = FT_LOAD_RENDER;
    if (isEmoji) {
        loadFlags |= FT_LOAD_COLOR;
    }

    FT_Error error = FT_Load_Char(face, codepoint, loadFlags);
    if (error) {
        return glyph; // Return empty glyph on error
    }

    FT_GlyphSlot slot = face->glyph;
    FT_Bitmap& bitmap = slot->bitmap;

    glyph.width = bitmap.width;
    glyph.height = bitmap.rows;
    glyph.bearingX = slot->bitmap_left;
    glyph.bearingY = slot->bitmap_top;
    glyph.advance = slot->advance.x >> 6;
    glyph.isColor = isEmoji && (bitmap.pixel_mode == FT_PIXEL_MODE_BGRA);

    // Return early for whitespace/empty glyphs
    if (glyph.width == 0 || glyph.height == 0) {
        return glyph;
    }

    // Create individual texture for this glyph (simplified approach)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &glyph.textureID);
    glBindTexture(GL_TEXTURE_2D, glyph.textureID);

    if (glyph.isColor) {
        // Color emoji - BGRA format
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glyph.width, glyph.height, 0,
                     GL_BGRA, GL_UNSIGNED_BYTE, bitmap.buffer);
    } else {
        // Regular text - grayscale
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, glyph.width, glyph.height, 0,
                     GL_RED, GL_UNSIGNED_BYTE, bitmap.buffer);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    return glyph;
}
void TextRenderer::RenderText(const std::string& text, float x, float y, float scale) {
    textShader->Use();
    
    // Use specific text projection matrix that handles coordinate system correctly
    Matrix4 projection = RenderUtils::CreateTextProjection(800.0f, 600.0f);
    textShader->SetUniformMatrix4fv("projection", projection.GetAsFloatPtr());
    textShader->SetUniform3f("textColor", mTextColor.x, mTextColor.y, mTextColor.z);
    
    RenderUtils::EnableBlending();
    RenderUtils::BindVAO(VAO);

    float cursorX = x;
    float cursorY = y;
    
    size_t pos = 0;
    while (pos < text.length()) {
        uint32_t codepoint = GetNextCodepoint(text, pos);
        if (codepoint == 0) break;
        
        // Check glyph cache
        auto it = glyphCache.find(codepoint);
        if (it == glyphCache.end()) {
            bool isEmoji = IsEmojiCodepoint(codepoint);
            
            FT_Face faceToUse = isEmoji ? fontManager->GetEmojiFace() : fontManager->GetTextFace();
            GlyphInfo glyph = LoadGlyph(codepoint, faceToUse, isEmoji);
            
            // Fallback to text font if emoji loading failed
            if (isEmoji && glyph.textureID == 0 && fontManager->GetEmojiFace() != fontManager->GetTextFace()) {
                glyph = LoadGlyph(codepoint, fontManager->GetTextFace(), false);
            }
            
            it = glyphCache.emplace(codepoint, glyph).first;
        }
        
        const GlyphInfo& glyph = it->second;
        
        if (glyph.textureID > 0) {
            // Scale and position calculations
            float emojiScale = glyph.isColor ? 0.35f : 1.0f;
            float scaled = scale * emojiScale;
            
            float xpos = cursorX + glyph.bearingX * scaled;
            float ypos = cursorY - glyph.bearingY * scaled;
            
            // Baseline adjustment for emojis
            if (glyph.isColor) {
                ypos -= (fontManager->GetTextFace()->size->metrics.height >> 6) * 0.15f * scale;
            }
            
            float w = glyph.width * scaled;
            float h = glyph.height * scaled;
            
            // Render quad vertices
            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 1.0f },
                { xpos,     ypos,       0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 0.0f },
                { xpos,     ypos + h,   0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 0.0f },
                { xpos + w, ypos + h,   1.0f, 1.0f }
            };
            
            textShader->SetUniform1i("isColorTexture", glyph.isColor ? 1 : 0);
            RenderUtils::BindTexture(glyph.textureID);
            
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        
        // Advance cursor
        float advanceScale = glyph.isColor ? 0.35f : 1.0f;
        cursorX += glyph.advance * scale * advanceScale;
    }
    
    RenderUtils::UnbindVAO();
    RenderUtils::UnbindTexture();
    RenderUtils::DisableBlending();
}

Vector2 TextRenderer::MeasureText(const std::string& text, float scale) const
{
    float totalWidth = 0.0f;
    float maxHeight = 0.0f;
    
    size_t pos = 0;
    while (pos < text.length())
    {
        uint32_t codepoint = GetNextCodepoint(text, pos);
        if (codepoint == 0) break;
        
        // Check glyph cache or load if needed
        auto it = glyphCache.find(codepoint);
        if (it == glyphCache.end())
        {
            bool isEmoji = IsEmojiCodepoint(codepoint);
            FT_Face faceToUse = isEmoji ? fontManager->GetEmojiFace() : fontManager->GetTextFace();
            GlyphInfo glyph = const_cast<TextRenderer*>(this)->LoadGlyph(codepoint, faceToUse, isEmoji);
            
            if (isEmoji && glyph.textureID == 0 && fontManager->GetEmojiFace() != fontManager->GetTextFace())
            {
                glyph = const_cast<TextRenderer*>(this)->LoadGlyph(codepoint, fontManager->GetTextFace(), false);
            }
            
            it = const_cast<TextRenderer*>(this)->glyphCache.emplace(codepoint, glyph).first;
        }
        
        const GlyphInfo& glyph = it->second;
        
        // Calculate width
        float emojiScale = glyph.isColor ? 0.35f : 1.0f;
        float scaled = scale * emojiScale;
        totalWidth += glyph.advance * scaled;
        
        // Track max height
        float glyphHeight = glyph.height * scaled;
        if (glyphHeight > maxHeight)
        {
            maxHeight = glyphHeight;
        }
    }
    
    return Vector2(totalWidth, maxHeight);
}

float TextRenderer::GetTextWidth(const std::string& text, float scale) const
{
    return MeasureText(text, scale).x;
}

float TextRenderer::GetTextHeight(const std::string& text, float scale) const
{
    return MeasureText(text, scale).y;
}

