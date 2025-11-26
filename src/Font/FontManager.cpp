#include "FontManager.hpp"
#include <iostream>

FontManager::FontManager()
    : ftLibrary(nullptr), textFace(nullptr), emojiFace(nullptr) {
}

FontManager::~FontManager() {
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

bool FontManager::Initialize() {
    if (FT_Init_FreeType(&ftLibrary)) {
        std::cerr << "ERROR: Could not initialize FreeType library!" << std::endl;
        return false;
    }

    return LoadFonts();
}

bool FontManager::LoadFonts() {
    if (!LoadTextFont()) {
        std::cerr << "ERROR: Could not load text font!" << std::endl;
        return false;
    }

    LoadEmojiFont(); // Emoji font is optional
    return true;
}

bool FontManager::LoadTextFont() {
    // Load text font - check multiple paths (prioritize local assets)
    std::vector<std::string> textPaths = {
        "../assets/third_party/Ninja Adventure - Asset Pack/Ui/Font/NormalFont.ttf",  // From build/ directory
        "assets/third_party/Ninja Adventure - Asset Pack/Ui/Font/NormalFont.ttf",      // From project root
        "../assets/third_party/NotoSans-Regular.ttf",  // Fallback: From build/ directory
        "assets/third_party/NotoSans-Regular.ttf",      // Fallback: From project root
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
    return true;
}

bool FontManager::LoadEmojiFont() {
    // Try to load emoji font - multiple paths (prioritize local assets)
    std::vector<std::string> emojiPaths = {
        "../assets/third_party/NotoColorEmoji-Regular.ttf",  // From build/ directory
        "assets/third_party/NotoColorEmoji-Regular.ttf",      // From project root
        "/usr/share/fonts/truetype/noto/NotoColorEmoji.ttf",  // System font
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
        FT_Set_Pixel_Sizes(emojiFace, 0, 48);

        if (emojiFace->num_fixed_sizes > 0) {
            int bestIndex = 0;
            int bestHeight = 0;
            for (int i = 0; i < emojiFace->num_fixed_sizes; ++i) {
                int h = emojiFace->available_sizes[i].height;
                if (h > bestHeight) { 
                    bestHeight = h; 
                    bestIndex = i; 
                }
            }
            FT_Select_Size(emojiFace, bestIndex);
        }
    } else {
        std::cout << "No emoji font found, using text font for emoji fallback" << std::endl;
        emojiFace = nullptr;
    }

    return true; // Emoji font is optional, so always return true
}