#include "SDLFont.hpp"
#include <iostream>
#include <algorithm>

const char* sdlVertexShaderSource = R"(
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

const char* sdlFragmentShaderSource = R"(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;

void main()
{
    // SDL2_ttf renders text as RGBA, so we can use it directly
    color = texture(text, TexCoords);
}
)";

SDLFont::SDLFont() : font(nullptr), fallbackFont(nullptr), VAO(0), VBO(0), shaderProgram(0) {
}

SDLFont::~SDLFont() {
    // Clean up textures
    for (auto& pair : textCache) {
        glDeleteTextures(1, &pair.second.textureID);
    }
    
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);
    
    if (font) TTF_CloseFont(font);
    if (fallbackFont) TTF_CloseFont(fallbackFont);
}

bool SDLFont::Initialize() {
    if (TTF_Init() != 0) {
        std::cout << "ERROR::SDL_TTF: Could not initialize SDL_ttf: " << TTF_GetError() << std::endl;
        return false;
    }
    
    return CreateShaders();
}

bool SDLFont::LoadFont(const std::string& fontPath, int fontSize) {
    // Load regular font as primary
    font = TTF_OpenFont("assets/NotoSans-Regular.ttf", fontSize);
    if (!font) {
        // Try system fonts as fallback
        font = TTF_OpenFont("/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf", fontSize);
    }
    
    if (!font) {
        std::cout << "ERROR::SDL_TTF: Failed to load regular font" << std::endl;
        return false;
    }
    
    // Load emoji font as fallback for emojis
    fallbackFont = TTF_OpenFont(fontPath.c_str(), fontSize);
    if (!fallbackFont) {
        std::cout << "WARNING::SDL_TTF: Failed to load emoji font " << fontPath << ": " << TTF_GetError() << std::endl;
        std::cout << "Emojis will render as white squares" << std::endl;
    } else {
        std::cout << "Successfully loaded emoji font: " << fontPath << std::endl;
    }
    
    std::cout << "Successfully loaded main font (NotoSans-Regular)" << std::endl;
    return true;
}

bool SDLFont::CreateShaders() {
    // Compile vertex shader
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &sdlVertexShaderSource, NULL);
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
    glShaderSource(fragment, 1, &sdlFragmentShaderSource, NULL);
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

SDL_Surface* SDLFont::RenderTextSurface(const std::string& text) {
    SDL_Color textColor = {255, 255, 255, 255}; // White text
    
    std::cout << "Attempting to render text: '" << text << "'" << std::endl;
    std::cout << "Main font available: " << (font ? "YES" : "NO") << std::endl;
    std::cout << "Fallback font available: " << (fallbackFont ? "YES" : "NO") << std::endl;
    
    SDL_Surface* textSurface = nullptr;
    
    // Check if text contains emojis - try emoji font first if we have it
    bool hasEmoji = false;
    for (char c : text) {
        if (static_cast<unsigned char>(c) > 127) { // Non-ASCII characters (potential emojis)
            hasEmoji = true;
            break;
        }
    }
    
    std::cout << "Text has emoji characters: " << (hasEmoji ? "YES" : "NO") << std::endl;
    
    if (hasEmoji && fallbackFont) {
        std::cout << "Text contains emojis, trying emoji font first..." << std::endl;
        // For color emojis, try different rendering methods
        textSurface = TTF_RenderUTF8_Blended(fallbackFont, text.c_str(), textColor);
        if (!textSurface) {
            std::cout << "Blended failed, trying Solid..." << std::endl;
            textSurface = TTF_RenderUTF8_Solid(fallbackFont, text.c_str(), textColor);
        }
        if (!textSurface) {
            std::cout << "Solid failed, trying Shaded..." << std::endl;
            SDL_Color bgColor = {0, 0, 0, 0}; // Transparent background
            textSurface = TTF_RenderUTF8_Shaded(fallbackFont, text.c_str(), textColor, bgColor);
        }
        
        if (textSurface) {
            std::cout << "Successfully rendered with emoji font. Surface: " << textSurface->w << "x" << textSurface->h << std::endl;
            return textSurface;
        } else {
            std::cout << "All emoji font methods failed: " << TTF_GetError() << std::endl;
        }
    }
    
    // Try main (regular) font
    textSurface = TTF_RenderUTF8_Blended(font, text.c_str(), textColor);
    
    if (textSurface) {
        std::cout << "Successfully rendered with main font. Surface: " << textSurface->w << "x" << textSurface->h << std::endl;
    } else {
        std::cout << "Main font failed: " << TTF_GetError() << std::endl;
    }
    
    if (!textSurface) {
        std::cout << "ERROR::SDL_TTF: Unable to render text '" << text << "': " << TTF_GetError() << std::endl;
    }
    
    return textSurface;
}

GLuint SDLFont::CreateTextureFromSurface(SDL_Surface* surface) {
    // Convert surface to RGBA32 for OpenGL
    SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    if (!converted) {
        std::cout << "ERROR: SDL_ConvertSurfaceFormat failed: " << SDL_GetError() << std::endl;
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, converted->w, converted->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, converted->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    SDL_FreeSurface(converted);
    return texture;
}

void SDLFont::CacheText(const std::string& text) {
    if (textCache.find(text) != textCache.end()) {
        std::cout << "Text '" << text << "' already cached" << std::endl;
        return; // Already cached
    }
    
    std::cout << "Caching text: '" << text << "'" << std::endl;
    
    SDL_Surface* textSurface = RenderTextSurface(text);
    if (!textSurface) {
        std::cout << "Failed to create surface for text: '" << text << "'" << std::endl;
        return;
    }
    
    std::cout << "Creating texture from surface..." << std::endl;
    GLuint texture = CreateTextureFromSurface(textSurface);
    
    SDLCharacter character = {
        texture,
        textSurface->w,
        textSurface->h,
        0, // bearingX
        textSurface->h, // bearingY
        textSurface->w // advance
    };
    
    textCache[text] = character;
    std::cout << "Cached text '" << text << "' with texture ID: " << texture << std::endl;
    
    SDL_FreeSurface(textSurface);
}

void SDLFont::RenderText(const std::string& text, float x, float y, float scale) {
    std::cout << "RenderText called for: '" << text << "' at (" << x << ", " << y << ")" << std::endl;
    
    // Cache the text if not already cached
    CacheText(text);
    
    auto it = textCache.find(text);
    if (it == textCache.end()) {
        std::cout << "Text not found in cache after caching attempt!" << std::endl;
        return; // Failed to cache/render text
    }
    
    SDLCharacter& ch = it->second;
    std::cout << "Found cached text. Dimensions: " << ch.width << "x" << ch.height << ", textureID: " << ch.textureID << std::endl;
    
    // Activate corresponding render state
    glUseProgram(shaderProgram);
    
    // Set projection matrix (orthographic projection for 800x600 window)
    // OpenGL orthographic projection: left=0, right=800, bottom=600, top=0
    float projection[16] = {
        2.0f/800.0f, 0.0f, 0.0f, 0.0f,
        0.0f, -2.0f/600.0f, 0.0f, 0.0f,  // Negative Y to flip coordinates  
        0.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f         // Adjusted translation
    };
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, projection);
    
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float xpos = x;
    float ypos = y;  // Start from the baseline position
    float w = ch.width * scale;
    float h = ch.height * scale;
    
    std::cout << "Rendering text '" << text << "' at position: (" << xpos << ", " << ypos << ") size: " << w << "x" << h << std::endl;
    
    // Update VBO for each character
    float vertices[6][4] = {
        { xpos,     ypos,       0.0f, 1.0f },  // Bottom left
        { xpos + w, ypos,       1.0f, 1.0f },  // Bottom right
        { xpos,     ypos + h,   0.0f, 0.0f },  // Top left

        { xpos + w, ypos,       1.0f, 1.0f },  // Bottom right  
        { xpos + w, ypos + h,   1.0f, 0.0f },  // Top right
        { xpos,     ypos + h,   0.0f, 0.0f }   // Top left           
    };
    
    // Render text texture over quad
    glBindTexture(GL_TEXTURE_2D, ch.textureID);
    
    // Update content of VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Render quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}