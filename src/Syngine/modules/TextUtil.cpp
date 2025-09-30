#include "TextUtil.hpp"

#include "Syngine/serialization/DataTemplates.hpp"
#include "freetype/freetype.h"

#include "Presets.hpp"
#include "freetype/fttypes.h"

#include <iostream>
#include <vector>

using namespace syng;

static FT_Library ft_lib = nullptr;

int TextUtil::init_FT2() {
    int ft_err = FT_Init_FreeType(&ft_lib);
    if (ft_err) {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library, errno=" << ft_err << std::endl;
        return ft_err;
    }
    return 0;
}

FT_Library TextUtil::getFT_Lib() {
    return ft_lib;
}

FT_Face TextUtil::newFT_Face(std::filesystem::path pathToFont) {
    FT_Face ft_face;
    int ft_err = FT_New_Face(ft_lib, pathToFont.c_str(), 0, &ft_face);
    if (ft_err) {
        std::cerr << "ERROR::FREETYPE: Failed to load font, errno=" << ft_err << std::endl;  
        return nullptr;
    }
    return ft_face;
}

FT_Face TextUtil::newFT_Face(DataDeserializer *buffer) {
    DataTemplates::push(buffer, "Font", PCK_HEADER_FT);
    uint64_t size = DataTemplates::read_uint64(buffer);
    std::vector<uint8_t> ft_base(size);
    buffer->read(ft_base.data(), size);
    DataTemplates::pop(buffer, "Font", PCK_FOOTER_FT);
    FT_Face ft_face;
    if (FT_New_Memory_Face(ft_lib, ft_base.data(), size, 0, &ft_face)) {
        std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;  
        return nullptr;
    }
    return ft_face;
}

void TextUtil::writeFT(DataSerializer *buffer, std::vector<FT_Byte> ft_base) {
    DataTemplates::write_uint16(buffer, PCK_HEADER_FT);
    DataTemplates::write_uint64(buffer, ft_base.size());
    buffer->write(ft_base.data(), ft_base.size());
    DataTemplates::write_uint16(buffer, PCK_FOOTER_FT);
}

TextUtil::T_Char generateGlyph(FT_Face ft_face, int32_t c, GLenum filter_type) {
    if (FT_Load_Char(ft_face, c, FT_LOAD_RENDER)) {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph for character '" << c << "'" << std::endl;
        return {0, {0, 0}, {0, 0}, 0};
    }
    GLuint TCB;
    glGenTextures(1, &TCB);
    glBindTexture(GL_TEXTURE_2D, TCB);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        ft_face->glyph->bitmap.width,
        ft_face->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        ft_face->glyph->bitmap.buffer
    );
    
    PresetsTexel::TextureParamST(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE);
    PresetsTexel::TextureFilter(GL_TEXTURE_2D, filter_type);
    
    return {
        TCB, 
        glm::ivec2(ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows),
        glm::ivec2(ft_face->glyph->bitmap_left, ft_face->glyph->bitmap_top),
        ft_face->glyph->advance.x
    };
}

std::unordered_map<char, TextUtil::T_Char> TextUtil::generateBitmap(FT_Face ft_face, std::vector<char> chars, GLenum filter_type) {
    if (!ft_face) {
        std::cerr << "ERROR::FT_FACE Cannot be nullptr\n";
        return {};
    }
    std::unordered_map<char, TextUtil::T_Char> result;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    for (char c : chars) {
        TextUtil::T_Char tc = generateGlyph(ft_face, c, filter_type);
        if (tc.TCB) result.insert({c, tc});
    }
    return result;
}

std::unordered_map<wchar_t, TextUtil::T_Char> TextUtil::generateBitmap_Unicode(FT_Face ft_face, std::vector<wchar_t> chars, GLenum filter_type) {
    if (!ft_face) {
        std::cerr << "ERROR::FT_FACE Cannot be nullptr\n";
        return {};
    }
    std::unordered_map<wchar_t, TextUtil::T_Char> result;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    for (wchar_t c : chars) {
        TextUtil::T_Char tc = generateGlyph(ft_face, c, filter_type);
        if (tc.TCB) result.insert({c, tc});
    }
    return result;
}