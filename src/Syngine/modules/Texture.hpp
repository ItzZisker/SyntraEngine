#pragma once

#include "Syngine/serialization/DataSerializer.hpp"

#include <glad/glad.h>

#include <filesystem>
#include <future>
#include <array>
#include <vector>
#include <string>

namespace syng
{

constexpr uint16_t PCK_HEADER_TEX2D = 200;
constexpr uint16_t PCK_FOOTER_TEX2D = 201;

constexpr uint16_t PCK_HEADER_TEX2D_MESH = 202;
constexpr uint16_t PCK_FOOTER_TEX2D_MESH = 203;

constexpr uint16_t PCK_HEADER_TEXQBMP = 204;
constexpr uint16_t PCK_FOOTER_TEXQBMP = 205;

struct Texture2D {
    GLuint TCB = 0;
    std::string path;
};
struct TextureCubemap {
    GLuint TCB = 0;
    std::string paths[6];
};

enum MeshTexture2D_T {
    Texture_Diffuse,
    Texture_Specular,
    Texture_Normal,
    Texture_Height,
    Texture_Rough
};
struct MeshTexture2D {
    Texture2D texture;
    MeshTexture2D_T type;
};

class TextureWriter {
private:
    DataSerializer *buffer;
public:
    TextureWriter(DataSerializer *buffer);

    void writeTexture2D(Texture2D tex2D);
    void writeTexture2D(std::string path, std::vector<uint8_t> bytes);

    void writeTextureCubemap(TextureCubemap texQB);
    void writeTextureCubemap(std::string paths[6], std::vector<uint8_t> bytes[6]);

    void writeMeshTexture2D(MeshTexture2D meshTex2D);
    void writeMeshTexture2D(std::string path, std::vector<uint8_t> bytes, MeshTexture2D_T type);
};

constexpr std::array<const char*, 5> TextureTNames = {
    "texture_diffuse",
    "texture_specular",
    "texture_normal",
    "texture_height",
    "texture_roughness"
};
constexpr const char* TEXTURE_NAME(MeshTexture2D_T type) {
    auto i = static_cast<size_t>(type);
    if (i >= TextureTNames.size()) {
        return "Unknown";
    }
    return TextureTNames[i];
}

std::future<GLuint> TCBFromBytes_TQ(uint8_t *raw, int width, int height, int nrComponents);
GLuint TCBFromBytes(uint8_t *raw, int width, int height, int nrComponents);
GLuint TCBFromFileBytes(unsigned char *bytes, int length);
GLuint TCBFromFile(const std::filesystem::path& path);

std::future<GLuint> QBTCBFromBytes_TQ(uint8_t **raws, int *widths, int *heights, int *nrComponentss);
GLuint QBTCBFromBytes(uint8_t **raw, int widths[6], int heights[6], int nrComponents[6]);
GLuint QBTCBFromFilesBytes(unsigned char **bytes, int lengths[6]);
GLuint QBTCBFromFiles(std::vector<std::filesystem::path>& paths);

Texture2D loadTexture2D(const std::filesystem::path& path);
Texture2D loadTexture2D(DataDeserializer *buffer);

TextureCubemap loadTextureCubemap(std::vector<std::filesystem::path> paths);
TextureCubemap loadTextureCubemap(DataDeserializer *buffer);

MeshTexture2D loadMeshTexture2D(const std::filesystem::path& path, MeshTexture2D_T type);
MeshTexture2D loadMeshTexture2D(DataDeserializer *buffer);

}