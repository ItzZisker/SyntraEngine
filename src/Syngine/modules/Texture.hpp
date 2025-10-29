#pragma once

#include "Syngine/ports/GLPort.h"
#include "Syngine/serialization/DataSerializer.hpp"

#ifdef USE_ASSIMP
#include "assimp/material.h"
#endif

#include <filesystem>
#include <functional>
#include <future>
#include <array>
#include <vector>
#include <string>

namespace syng
{

constexpr uint16_t PCK_HEADER_TEX2D = 104;
constexpr uint16_t PCK_FOOTER_TEX2D = 105;

constexpr uint16_t PCK_HEADER_TEX2D_MESH = 106;
constexpr uint16_t PCK_FOOTER_TEX2D_MESH = 107;

constexpr uint16_t PCK_HEADER_TEXQBMP = 108;
constexpr uint16_t PCK_FOOTER_TEXQBMP = 109;

struct TextureImage {
    std::vector<uint8_t> bytes;
    int width, height, nrComponents;

    size_t getSize() { return width * height * nrComponents; }
};

using TexelExecParams = std::function<void(GLuint TCB)>;

inline const TexelExecParams PARAMS_TEX2D_DEFAULT = [](GLuint TCB) {
#ifdef __EMSCRIPTEN__
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#else
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
};

inline const TexelExecParams PARAMS_TEXCUBE_DEFAULT = [](GLuint TCB) {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
};

class Texture2D {
private:
    TextureImage image;
    GLuint TCB = 0;
    std::string path;
public:
    Texture2D(GLuint TCB);
    Texture2D(TextureImage image = {}, std::string path = {});

    void setTCB(GLuint TCB);
    GLuint getTCB() const;
    const std::string& getPath() const;

    void readImage();
    void clearImage();

    bool isUploaded();
    void deleteTexture();
    void uploadTexture(TexelExecParams exec_params = PARAMS_TEX2D_DEFAULT);
};

class TextureCubemap {
private:
    std::array<TextureImage, 6> images;
    GLuint TCB = 0;
    std::array<std::string, 6> paths;
public:
    TextureCubemap(GLuint TCB);
    TextureCubemap(std::array<TextureImage, 6> images = {}, std::array<std::string, 6> paths = {});
    ~TextureCubemap();

    void setTCB(GLuint TCB);
    GLuint getTCB() const;
    const std::string& getPath(GLenum face) const;

    void readImages();
    void clearImages();

    bool isUploaded();
    void deleteTexture();
    void uploadTexture(TexelExecParams exec_params = PARAMS_TEXCUBE_DEFAULT);
};

enum MaterialTexture2D_T {
    Texture_Diffuse,
    Texture_Specular,
    Texture_Normal,
    Texture_Height,
    Texture_Rough,
    Texture_Metallic, // Required for PBR
    Texture_AmbientOcclusion,
    Texture_Emissive
};

namespace TextureIO {
    class TexturePackedWriter {
    private:
        DataSerializer *buffer;
    public:
        TexturePackedWriter(DataSerializer *buffer);

        void writeTexture2D(Texture2D tex2D);
        void writeTexture2D(std::string path, TextureImage& image);

        void writeTextureCubemap(TextureCubemap texQB);
        void writeTextureCubemap(std::string paths[6], TextureImage images[6]);

        void writeMeshTexture2D(MaterialTexture2D_T texType, Texture2D texel);
        void writeMeshTexture2D(std::string path, TextureImage& image, MaterialTexture2D_T type);
    };

    class TexturePackedReader {
    private:
        DataDeserializer *buffer;
    public:
        TexturePackedReader(DataDeserializer *buffer);

        Texture2D readTexture2D();
        TextureCubemap readTextureCubemap();
        std::pair<MaterialTexture2D_T, Texture2D> readMeshTexture2D();
    };

    namespace TextureFileReader {
        Texture2D readTexture2D(const std::filesystem::path& path);
        TextureCubemap readTextureCubemap(std::vector<std::filesystem::path> paths);
    };
}

// enum MaterialTexture2D_T {
//     Texture_Diffuse,
//     Texture_Specular,
//     Texture_Normal,
//     Texture_Height,
//     Texture_Rough,
//     Texture_Metallic, // Required for PBR
//     Texture_AmbientOcclusion,
//     Texture_Emissive
// };

constexpr std::array<const char*, 8> TextureTNames = {
    "texture_diffuse",
    "texture_specular",
    "texture_normal",
    "texture_height",
    "texture_roughness",
    "texture_metallic",
    "texture_ao",
    "texture_emissive"

};
constexpr const char* TEXTURE_NAME(MaterialTexture2D_T type) {
    auto i = static_cast<size_t>(type);
    if (i >= TextureTNames.size()) {
        return "Unknown";
    }
    return TextureTNames[i];
}

#ifdef USE_ASSIMP
constexpr std::array<const aiTextureType, 8> TextureTAssimp = {
    aiTextureType_DIFFUSE,
    aiTextureType_SPECULAR,
    aiTextureType_NORMALS,
    aiTextureType_HEIGHT,
    aiTextureType_DIFFUSE_ROUGHNESS,
    aiTextureType_METALNESS,
    aiTextureType_AMBIENT_OCCLUSION,
    aiTextureType_EMISSIVE
};
constexpr const aiTextureType TEXTURE_ASSIMP(MaterialTexture2D_T type) {
    auto i = static_cast<size_t>(type);
    if (i >= TextureTNames.size()) {
        return aiTextureType_NONE;
    }
    return TextureTAssimp[i];
}
#endif

TextureImage readTextureImage(const std::filesystem::path& path);

GLuint TCBByPlainColor(unsigned char pixel[4]);
void TCBPlainColor(unsigned int &TCB, unsigned char pixel[4]);

std::future<GLuint> uploadTex2DFromBytes_TQ(uint8_t *raw, int width, int height, int nrComponents, TexelExecParams exec_params);
GLuint uploadTex2DFromBytes(uint8_t *raw, int width, int height, int nrComponents, TexelExecParams exec_params);
GLuint uploadTex2DFromFileBytes(unsigned char *bytes, int length, TexelExecParams exec_params);
GLuint uploadTex2DFromFile(const std::filesystem::path& path, TexelExecParams exec_params);

std::future<GLuint> uploadTexCubeFromBytes_TQ(uint8_t **raws, int *widths, int *heights, int *nrComponentss, TexelExecParams exec_params);
GLuint uploadTexCubeFromBytes(uint8_t **raw, int widths[6], int heights[6], int nrComponents[6], TexelExecParams exec_params);
GLuint uploadTexCubeFromFilesBytes(unsigned char **bytes, int lengths[6], TexelExecParams exec_params);
GLuint uploadTexCubeFromFiles(std::vector<std::filesystem::path>& paths, TexelExecParams exec_params);

}