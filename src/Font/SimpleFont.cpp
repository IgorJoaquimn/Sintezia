#include "SimpleFont.hpp"
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

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
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;
uniform int isColorTexture;

void main()
{
    vec4 sampled = texture(text, TexCoords);
    
    if (isColorTexture == 1) {
        // For color emoji (BGRA format), use all channels directly
        // The texture data is already in the correct format
        color = sampled;
        
        // Apply gamma correction for better color reproduction
        color.rgb = pow(color.rgb, vec3(1.0/2.2));
    } else {
        // For regular text, use red channel as alpha with textColor
        color = vec4(textColor * sampled.r, sampled.r);
    }
}
)";

SimpleFont::SimpleFont() : VAO(0), VBO(0), shaderProgram(0), library(nullptr), face(nullptr), fallbackFace(nullptr), mainFontType(FontType::REGULAR) {
}

SimpleFont::~SimpleFont() {
    // Clean up OpenGL resources
    for (auto& pair : characters) {
        glDeleteTextures(1, &pair.second.textureID);
    }
    for (auto& pair : unicodeCharacters) {
        glDeleteTextures(1, &pair.second.textureID);
    }
    
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);
    
    // Clean up FreeType faces and library
    if (face) {
        FT_Done_Face((FT_Face)face);
    }
    if (fallbackFace) {
        FT_Done_Face((FT_Face)fallbackFace);
    }
    if (library) {
        FT_Done_FreeType((FT_Library)library);
    }
}

bool SimpleFont::LoadFont(const std::string& fontPath, int fontSize) {
    // Initialize FreeType
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return false;
    }

    // Load font
    FT_Face ftFace;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &ftFace)) {
        std::cout << "ERROR::FREETYPE: Failed to load font from " << fontPath << std::endl;
        FT_Done_FreeType(ft);
        return false;
    }
    
    // Store library and face for later Unicode character loading
    this->library = ft;
    this->face = ftFace;

    // Detect font type and setup accordingly
    if (IsColorEmojiFont(ftFace)) {
        mainFontType = FontType::COLOR_EMOJI;
        SetupColorFont(ftFace, fontSize);
        std::cout << "Loaded color emoji font: " << fontPath << std::endl;
    } else {
        mainFontType = FontType::REGULAR;
        SetupRegularFont(ftFace, fontSize);
        std::cout << "Loaded regular font: " << fontPath << std::endl;
    }

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 characters of ASCII set
    for (unsigned char c = 0; c < 128; c++) {
        // Load character glyph
        if (FT_Load_Char(ftFace, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph " << (int)c << std::endl;
            continue;
        }
        
        // Skip characters with no bitmap (like spaces, control chars)
        if (ftFace->glyph->bitmap.width == 0 || ftFace->glyph->bitmap.rows == 0) {
            // For space character, create a simple placeholder
            if (c == ' ') {
                Character character = { 0, 0, 0, 0, 0, (int)ftFace->glyph->advance.x, false };
                characters.insert(std::pair<char, Character>(c, character));
            }
            continue;
        }
        
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            ftFace->glyph->bitmap.width,
            ftFace->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            ftFace->glyph->bitmap.buffer
        );
        
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Store character for later use
        Character character = {
            texture,
            (int)ftFace->glyph->bitmap.width,
            (int)ftFace->glyph->bitmap.rows,
            ftFace->glyph->bitmap_left,
            ftFace->glyph->bitmap_top,
            (int)ftFace->glyph->advance.x,
            false  // Regular ASCII characters are not color
        };
        characters.insert(std::pair<char, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create shaders and VAO/VBO
    if (!CreateShaders()) {
        return false;
    }

    std::cout << "Font loaded successfully: " << fontPath << std::endl;
    return true;
}

bool SimpleFont::LoadFallbackFont(const std::string& fontPath, int fontSize) {
    if (!library) {
        std::cout << "ERROR::FREETYPE: Main font must be loaded first" << std::endl;
        return false;
    }
    
    FT_Library ft = (FT_Library)library;
    FT_Face ftFace;
    
    if (FT_New_Face(ft, fontPath.c_str(), 0, &ftFace)) {
        std::cout << "ERROR::FREETYPE: Failed to load fallback font from " << fontPath << std::endl;
        return false;
    }
    
    // Store fallback face
    this->fallbackFace = ftFace;
    
    // Setup as regular font
    SetupRegularFont(ftFace, fontSize);
    
    std::cout << "Loaded fallback font: " << fontPath << std::endl;
    return true;
}


bool SimpleFont::CreateShaders() {
    // Compile vertex shader
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexShaderSource, NULL);
    glCompileShader(vertex);
    
    GLint success;
    GLchar infoLog[1024];
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: VERTEX\n" << infoLog << std::endl;
        return false;
    }

    // Compile fragment shader
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragment);
    
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: FRAGMENT\n" << infoLog << std::endl;
        return false;
    }

    // Create shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 1024, NULL, infoLog);
        std::cout << "ERROR::PROGRAM_LINKING_ERROR\n" << infoLog << std::endl;
        return false;
    }

    // Clean up shaders
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

void SimpleFont::RenderText(const std::string& text, float x, float y, float scale) {
    // Activate corresponding render state
    glUseProgram(shaderProgram);
    
    // Set text color (white)
    glUniform3f(glGetUniformLocation(shaderProgram, "textColor"), 1.0f, 1.0f, 1.0f);
    
    // Set projection matrix (orthographic projection for 800x600 window, corrected orientation)
    float projection[16] = {
        2.0f/800.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f/600.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f
    };
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, projection);
    
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // Enable blending for proper alpha compositing
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Convert UTF-8 text to Unicode codepoints
    std::vector<uint32_t> codepoints = UTF8ToCodepoints(text);
    
    // Iterate through all codepoints
    for (uint32_t codepoint : codepoints) {
        Character ch;
        bool found = false;
        
        // Check ASCII characters first (faster lookup)
        if (codepoint < 128) {
            auto it = characters.find((char)codepoint);
            if (it != characters.end()) {
                ch = it->second;
                found = true;
            }
        }
        
        // Check Unicode characters
        if (!found) {
            auto it = unicodeCharacters.find(codepoint);
            if (it != unicodeCharacters.end()) {
                ch = it->second;
                found = true;
            } else {
                // Try to load the character dynamically
                LoadUnicodeCharacter(codepoint);
                it = unicodeCharacters.find(codepoint);
                if (it != unicodeCharacters.end()) {
                    ch = it->second;
                    found = true;
                }
            }
        }
        
        if (!found) {
            // Character not found, skip
            continue;
        }

        float xpos = x + ch.bearingX * scale;
        float ypos = y - (ch.height - ch.bearingY) * scale;

        float w = ch.width * scale;
        float h = ch.height * scale;
        
        // Update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        
        // Set whether this is a color texture or not
        glUniform1i(glGetUniformLocation(shaderProgram, "isColorTexture"), ch.isColor ? 1 : 0);
        
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}

void SimpleFont::LoadUnicodeCharacter(uint32_t codepoint) {
    if (!face) return;
    
    // Check if character is already loaded
    if (unicodeCharacters.find(codepoint) != unicodeCharacters.end()) {
        return;
    }
    
    FT_Face ftFace = (FT_Face)face;
    
    // Simple approach - just load with FT_LOAD_RENDER
    FT_Error error = FT_Load_Char(ftFace, codepoint, FT_LOAD_RENDER);
    if (error) {
        // If loading fails, try fallback font
        if (fallbackFace) {
            FT_Face fallbackFtFace = (FT_Face)fallbackFace;
            error = FT_Load_Char(fallbackFtFace, codepoint, FT_LOAD_RENDER);
            if (error) {
                return; // Skip this character
            }
            ftFace = fallbackFtFace;
        } else {
            return; // Skip this character
        }
    }
    
    // Skip characters with no bitmap
    if (ftFace->glyph->bitmap.width == 0 || ftFace->glyph->bitmap.rows == 0) {
        // Create placeholder for space-like characters
        Character character = { 0, 0, 0, 0, 0, (int)ftFace->glyph->advance.x, false };
        unicodeCharacters[codepoint] = character;
        return;
    }
    
    // Generate texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Always create as grayscale texture (FreeType fallback)
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        ftFace->glyph->bitmap.width,
        ftFace->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        ftFace->glyph->bitmap.buffer
    );
    
    // Set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Store character
    Character character = {
        texture,
        (int)ftFace->glyph->bitmap.width,
        (int)ftFace->glyph->bitmap.rows,
        ftFace->glyph->bitmap_left,
        ftFace->glyph->bitmap_top,
        (int)ftFace->glyph->advance.x,
        false // Always grayscale for now
    };
    unicodeCharacters[codepoint] = character;
    
    glBindTexture(GL_TEXTURE_2D, 0);
}



std::vector<uint32_t> SimpleFont::UTF8ToCodepoints(const std::string& utf8) {
    std::vector<uint32_t> codepoints;
    
    for (size_t i = 0; i < utf8.length(); ) {
        uint32_t codepoint = 0;
        uint8_t c = utf8[i];
        
        if (c < 0x80) {
            // ASCII character
            codepoint = c;
            i++;
        } else if ((c >> 5) == 0x06) {
            // 2-byte sequence
            if (i + 1 < utf8.length()) {
                codepoint = ((c & 0x1f) << 6) | (utf8[i + 1] & 0x3f);
                i += 2;
            } else {
                i++;
            }
        } else if ((c >> 4) == 0x0e) {
            // 3-byte sequence
            if (i + 2 < utf8.length()) {
                codepoint = ((c & 0x0f) << 12) | ((utf8[i + 1] & 0x3f) << 6) | (utf8[i + 2] & 0x3f);
                i += 3;
            } else {
                i++;
            }
        } else if ((c >> 3) == 0x1e) {
            // 4-byte sequence (emojis are often here)
            if (i + 3 < utf8.length()) {
                codepoint = ((c & 0x07) << 18) | ((utf8[i + 1] & 0x3f) << 12) | ((utf8[i + 2] & 0x3f) << 6) | (utf8[i + 3] & 0x3f);
                i += 4;
            } else {
                i++;
            }
        } else {
            // Invalid sequence, skip
            i++;
            continue;
        }
        
        if (codepoint > 0) {
            codepoints.push_back(codepoint);
        }
    }
    
    return codepoints;
}

bool SimpleFont::IsColorEmojiFont(void* face) {
    FT_Face ftFace = (FT_Face)face;
    
    // Check for CBDT table (Color Bitmap Data Table) - used by emoji fonts
    static const FT_ULong CBDT_TAG = FT_MAKE_TAG('C', 'B', 'D', 'T');
    FT_ULong length = 0;
    FT_Error error = FT_Load_Sfnt_Table(ftFace, CBDT_TAG, 0, nullptr, &length);
    
    if (error == 0 && length > 0) {
        return true;
    }
    
    // Also check CBLC table (Color Bitmap Location Table)
    static const FT_ULong CBLC_TAG = FT_MAKE_TAG('C', 'B', 'L', 'C');
    error = FT_Load_Sfnt_Table(ftFace, CBLC_TAG, 0, nullptr, &length);
    
    return (error == 0 && length > 0);
}

void SimpleFont::SetupColorFont(void* face, int fontSize) {
    FT_Face ftFace = (FT_Face)face;
    
    // Check for the correct way to size your fonts
    if (FT_HAS_FIXED_SIZES(ftFace)) {
        FT_Select_Size(ftFace, 0); // just use the first one
        std::cout << "Selected emoji size: " << ftFace->available_sizes[0].width << "px" << std::endl;
    } else {
        // The usual way to set the size
        FT_Set_Pixel_Sizes(ftFace, 0, fontSize);
    }
}

void SimpleFont::SetupRegularFont(void* face, int fontSize) {
    FT_Face ftFace = (FT_Face)face;
    FT_Set_Pixel_Sizes(ftFace, 0, fontSize);
}