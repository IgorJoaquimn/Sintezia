#include "TextRenderer.hpp"
#include <iostream>
#include <cstring>

TextRenderer::TextRenderer()
    : ftLibrary(nullptr), textFace(nullptr), emojiFace(nullptr),
      atlasTexture(0), atlasWidth(1024), atlasHeight(1024),
      currentX(0), currentY(0), rowHeight(0), VAO(0), VBO(0), shaderProgram(0) {
}

TextRenderer::~TextRenderer() {
    if (atlasTexture) {
        glDeleteTextures(1, &atlasTexture);
    }

    // Clean up glyph textures
    for (auto& pair : glyphCache) {
        if (pair.second.textureID) {
            glDeleteTextures(1, &pair.second.textureID);
        }
    }

    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);

    if (textFace) {
        FT_Done_Face(textFace);
    }

    if (emojiFace) {
        FT_Done_Face(emojiFace);
    }

    if (ftLibrary) {
        FT_Done_FreeType(ftLibrary);
    }
}

bool TextRenderer::Initialize() {
    // Initialize FreeType once
    if (FT_Init_FreeType(&ftLibrary)) {
        std::cout << "ERROR: Could not initialize FreeType library!" << std::endl;
        return false;
    }

    if (!LoadFonts()) {
        std::cout << "ERROR: Could not load fonts!" << std::endl;
        return false;
    }

    if (!CreateAtlas()) {
        std::cout << "ERROR: Could not create texture atlas!" << std::endl;
        return false;
    }

    if (!CreateShaders()) {
        std::cout << "ERROR: Could not create shaders!" << std::endl;
        return false;
    }

    // Create VAO/VBO (ensure done after shader creation if attribute locations are assumed)
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Enable blending once
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::cout << "TextRenderer initialized with color emoji support" << std::endl;
    return true;
}

bool TextRenderer::LoadFonts() {
    // Assumes FreeType already initialized by Initialize()

    // Load text font - check multiple paths
    std::vector<std::string> textPaths = {
        "assets/NotoSans-Regular.ttf",
        "../assets/NotoSans-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf"
    };

    bool textLoaded = false;
    for (const auto& path : textPaths) {
        if (FT_New_Face(ftLibrary, path.c_str(), 0, &textFace) == 0) {
            std::cout << "Successfully loaded text font: " << path << std::endl;
            textLoaded = true;
            break;
        }
    }

    if (!textLoaded) {
        std::cerr << "ERROR::FREETYPE: Failed to load text font from any path" << std::endl;
        return false;
    }

    FT_Set_Pixel_Sizes(textFace, 0, 48);

    // Try to load emoji font - multiple paths
    std::vector<std::string> emojiPaths = {
        "/usr/share/fonts/truetype/noto/NotoColorEmoji.ttf",  // System font first
        "assets/NotoColorEmoji-Regular.ttf",
        "../assets/NotoColorEmoji-Regular.ttf",
        "/System/Library/Fonts/Apple Color Emoji.ttc",
        "/usr/share/fonts/TTF/NotoColorEmoji.ttf"
    };

    bool emojiLoaded = false;
    for (const auto& path : emojiPaths) {
        if (FT_New_Face(ftLibrary, path.c_str(), 0, &emojiFace) == 0) {
            std::cout << "Successfully loaded emoji font: " << path << std::endl;
            emojiLoaded = true;
            break;
        }
    }

    if (emojiLoaded) {
        // Set emoji font to same size as text font for consistency
        FT_Set_Pixel_Sizes(emojiFace, 0, 48);

        if (emojiFace->num_fixed_sizes > 0) {
            std::cout << "Color emoji font has " << emojiFace->num_fixed_sizes << " fixed sizes" << std::endl;
            int bestIndex = 0;
            int bestHeight = 0;
            for (int i = 0; i < emojiFace->num_fixed_sizes; ++i) {
                int h = emojiFace->available_sizes[i].height;
                if (h > bestHeight) { bestHeight = h; bestIndex = i; }
            }
            FT_Select_Size(emojiFace, bestIndex);
            std::cout << "Selected emoji size: " << bestHeight << "px" << std::endl;
        }
    } else {
        std::cout << "No emoji font found, using text font for all characters" << std::endl;
        emojiFace = nullptr;
    }

    return true;
}

bool TextRenderer::CreateShaders() {
    const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

    const char* fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D text;
uniform vec3 textColor;
uniform int isColorTexture;

void main()
{
    vec4 sampled = texture(text, TexCoords);
    if (isColorTexture == 1) {
        // Color texture already contains alpha premultiplied if provided by font bitmap
        FragColor = sampled;
    } else {
        // Regular text: sampled.r holds the mask
        FragColor = vec4(textColor * sampled.r, sampled.r);
    }
}
)";

    auto compile = [](GLenum type, const char* src) -> GLuint {
        GLuint sh = glCreateShader(type);
        glShaderSource(sh, 1, &src, NULL);
        glCompileShader(sh);
        GLint ok = 0;
        glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLchar info[1024];
            glGetShaderInfoLog(sh, 1024, NULL, info);
            std::cout << "Shader compile error: " << info << std::endl;
            glDeleteShader(sh);
            return 0;
        }
        return sh;
    };

    GLuint v = compile(GL_VERTEX_SHADER, vertexShaderSource);
    if (!v) return false;
    GLuint f = compile(GL_FRAGMENT_SHADER, fragmentShaderSource);
    if (!f) { glDeleteShader(v); return false; }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, v);
    glAttachShader(shaderProgram, f);
    glLinkProgram(shaderProgram);

    GLint ok = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLchar info[1024];
        glGetProgramInfoLog(shaderProgram, 1024, NULL, info);
        std::cout << "Shader link error: " << info << std::endl;
        glDeleteShader(v);
        glDeleteShader(f);
        return false;
    }

    glDeleteShader(v);
    glDeleteShader(f);
    return true;
}

bool TextRenderer::CreateAtlas() {
    glGenTextures(1, &atlasTexture);
    glBindTexture(GL_TEXTURE_2D, atlasTexture);

    // Create empty atlas texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlasWidth, atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

AtlasRegion TextRenderer::AllocateAtlasRegion(int width, int height) {
    // Simple row-based atlas allocation
    if (currentX + width > atlasWidth) {
        currentX = 0;
        currentY += rowHeight;
        rowHeight = 0;
    }

    if (currentY + height > atlasHeight) {
        // Atlas is full - for now just fail
        return {-1, -1, 0, 0};
    }

    AtlasRegion region = {currentX, currentY, width, height};
    currentX += width;
    rowHeight = std::max(rowHeight, height);

    return region;
}

uint32_t TextRenderer::GetNextCodepoint(const std::string& text, size_t& pos) {
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
    glyph.textureID = 0; // ensure initialized

    // Load glyph with color support for emojis
    FT_Int32 loadFlags = FT_LOAD_RENDER;
    if (isEmoji) {
        loadFlags |= FT_LOAD_COLOR;
    }

    FT_Error error = FT_Load_Char(face, codepoint, loadFlags);
    if (error) {
        std::cout << "FT_Load_Char failed for codepoint " << codepoint << " with error: " << error << std::endl;
        return glyph; // Return empty glyph on error
    }

    FT_GlyphSlot slot = face->glyph;
    FT_Bitmap& bitmap = slot->bitmap;

    std::cout << "Glyph loaded - codepoint: " << codepoint << ", bitmap: " << bitmap.width << "x" << bitmap.rows << ", pixel_mode: " << (int)bitmap.pixel_mode << std::endl;

    glyph.width = bitmap.width;
    glyph.height = bitmap.rows;
    glyph.bearingX = slot->bitmap_left;
    glyph.bearingY = slot->bitmap_top;
    glyph.advance = slot->advance.x >> 6;
    glyph.isColor = isEmoji && (bitmap.pixel_mode == FT_PIXEL_MODE_BGRA);

    // If whitespace / empty glyph, return glyph with advance but no texture
    if (glyph.width == 0 || glyph.height == 0) {
        return glyph;
    }

    // Allocate space in atlas (for now we create per-glyph textures)
    AtlasRegion region = AllocateAtlasRegion(glyph.width, glyph.height);
    if (region.x == -1) {
        std::cout << "Atlas full for codepoint " << codepoint << std::endl;
        return glyph; // Atlas full
    }

    // Create texture for this glyph
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // important for tightly packed data
    glGenTextures(1, &glyph.textureID);
    glBindTexture(GL_TEXTURE_2D, glyph.textureID);

    if (glyph.isColor) {
        // Color emoji - BGRA format from FreeType
        std::cout << "Creating color texture for codepoint " << codepoint << std::endl;
        // Use GL_RGBA as internal format and GL_BGRA for the source format
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glyph.width, glyph.height, 0,
                     GL_BGRA, GL_UNSIGNED_BYTE, bitmap.buffer);
    } else {
        // Regular text - grayscale in bitmap.buffer
        std::cout << "Creating grayscale texture for codepoint " << codepoint << std::endl;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, glyph.width, glyph.height, 0,
                     GL_RED, GL_UNSIGNED_BYTE, bitmap.buffer);
        // No swizzle needed - we'll use the red channel directly in the shader
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    return glyph;
}
void TextRenderer::RenderText(const std::string& text, float x, float y, float scale) {
    std::cout << "RenderText called: '" << text << "' at (" << x << ", " << y << ")" << std::endl;
    
    // Activate shader program
    glUseProgram(shaderProgram);
    
    // Set projection matrix (orthographic projection for 800x600 window)
    // Standard 2D projection: left=0, right=800, bottom=0, top=600
    float projection[16] = {
        2.0f/800.0f, 0.0f, 0.0f, 0.0f,
        0.0f, -2.0f/600.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, projection);
    
    // Set text color (white)
    glUniform3f(glGetUniformLocation(shaderProgram, "textColor"), 1.0f, 1.0f, 1.0f);
    
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float cursorX = x;
    float cursorY = y;
    
    size_t pos = 0;
    while (pos < text.length()) {
        uint32_t codepoint = GetNextCodepoint(text, pos);
        if (codepoint == 0) break;
        
        // Check if we have this glyph cached
        auto it = glyphCache.find(codepoint);
        if (it == glyphCache.end()) {
            // Determine if this is likely an emoji (expanded heuristic)
            bool isEmoji = (codepoint >= 0x1F300 && codepoint <= 0x1F9FF) ||  // Misc symbols and pictographs
                          (codepoint >= 0x1F600 && codepoint <= 0x1F64F) ||  // Emoticons
                          (codepoint >= 0x1F680 && codepoint <= 0x1F6FF) ||  // Transport and map symbols
                          (codepoint >= 0x2600 && codepoint <= 0x26FF) ||   // Misc symbols
                          (codepoint >= 0x2700 && codepoint <= 0x27BF) ||   // Dingbats
                          (codepoint >= 0x1F1E0 && codepoint <= 0x1F1FF);   // Regional indicator symbols
            
            FT_Face faceToUse = isEmoji ? emojiFace : textFace;
            GlyphInfo glyph = LoadGlyph(codepoint, faceToUse, isEmoji);
            
            // If emoji loading failed, try with text font as fallback
            if (isEmoji && glyph.textureID == 0 && emojiFace != textFace) {
                std::cout << "Emoji failed, trying text font for codepoint " << codepoint << std::endl;
                glyph = LoadGlyph(codepoint, textFace, false);
            }
            
            std::cout << "Loaded glyph for codepoint " << codepoint << " (char: " << (char)codepoint << "), textureID: " << glyph.textureID << ", size: " << glyph.width << "x" << glyph.height << std::endl;
            
            it = glyphCache.emplace(codepoint, glyph).first;
        }
        
        const GlyphInfo& glyph = it->second;
        
        if (glyph.textureID > 0) {
            // Scale emojis smaller and align baseline
            float emojiScale = glyph.isColor ? 0.35f : 1.0f;
            float scaled = scale * emojiScale;
            
            // Calculate glyph position with proper baseline handling
            float xpos = cursorX + glyph.bearingX * scaled;
            float ypos = cursorY - glyph.bearingY * scaled;
            
            // Baseline adjustment for emojis
            if (glyph.isColor) {
                ypos -= (textFace->size->metrics.height >> 6) * 0.15f * scale;  // Raise emojis up
            }
            
            float w = glyph.width * scaled;
            float h = glyph.height * scaled;
            
            // Update VBO for each character with correct texture coordinates
            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 1.0f },  // Top left
                { xpos,     ypos,       0.0f, 0.0f },  // Bottom left
                { xpos + w, ypos,       1.0f, 0.0f },  // Bottom right

                { xpos,     ypos + h,   0.0f, 1.0f },  // Top left
                { xpos + w, ypos,       1.0f, 0.0f },  // Bottom right
                { xpos + w, ypos + h,   1.0f, 1.0f }   // Top right
            };
            
            // Set color/emoji texture flag
            glUniform1i(glGetUniformLocation(shaderProgram, "isColorTexture"), glyph.isColor ? 1 : 0);
            
            // Render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, glyph.textureID);
            
            // Update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            
            // Render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        
        // Advance cursor with proper scaling for emojis
        float advanceScale = glyph.isColor ? 0.35f : 1.0f;
        cursorX += glyph.advance * scale * advanceScale;
    }
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}

