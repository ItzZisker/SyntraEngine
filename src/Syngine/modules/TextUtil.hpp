#pragma once

#include <ft2build.h>
#include <freetype/freetype.h>

#include "Syngine/serialization/DataSerializer.hpp"

#include "glm/glm.hpp"

#include <glad/glad.h>

#include <unordered_map>
#include <filesystem>
#include <string>
#include <vector>

namespace syng
{
namespace TextUtil
{

constexpr uint16_t PCK_HEADER_FT = 110;
constexpr uint16_t PCK_FOOTER_FT = 111;

static const std::string UNIVERSAL_CHARS = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

struct T_Char {
    GLuint TCB;
    glm::ivec2 size;
    glm::ivec2 bearing;
    long advance;
};

int init_FT2();
FT_Library getFT_Lib();

FT_Face newFT_Face(std::filesystem::path pathToFont);
FT_Face newFT_Face(DataDeserializer *buffer);
void writeFT(DataSerializer *buffer, std::vector<FT_Byte> ft_base);

std::unordered_map<char, T_Char> generateBitmap(
    FT_Face ft_face,
    std::vector<char> chars = std::vector(UNIVERSAL_CHARS.begin(), UNIVERSAL_CHARS.end()),
    GLenum filter_type = GL_LINEAR
);
std::unordered_map<wchar_t, T_Char> generateBitmap_Unicode(
    FT_Face ft_face,
    std::vector<wchar_t> chars,
    GLenum filter_type = GL_LINEAR
);

}
}