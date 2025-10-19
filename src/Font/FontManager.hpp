#ifndef FONTMANAGER_HPP
#define FONTMANAGER_HPP

#include <ft2build.h>
#include FT_FREETYPE_H
#include <string>
#include <vector>

class FontManager {
public:
    FontManager();
    ~FontManager();

    bool Initialize();
    bool LoadFonts();
    
    FT_Face GetTextFace() const { return textFace; }
    FT_Face GetEmojiFace() const { return emojiFace; }
    FT_Library GetFTLibrary() const { return ftLibrary; }

private:
    FT_Library ftLibrary;
    FT_Face textFace;
    FT_Face emojiFace;

    bool LoadTextFont();
    bool LoadEmojiFont();
};

#endif // FONTMANAGER_HPP