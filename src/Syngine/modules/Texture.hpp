#pragma once

#include "Syngine/ports/GLPort.h"
#include "Syngine/serialization/DataSerializer.hpp"
#include <cstddef>

#ifdef USE_ASSIMP
#include "assimp/material.h"
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/component_wise.hpp>

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

struct ImageFormatInfo {
    GLenum internalFormat;
    GLenum format;
    GLenum type;
    int nrComponents;

    size_t bits() const {
        return type == GL_HALF_FLOAT ? 2 : type == GL_FLOAT ? 4 : 1;
    }
};

enum ImageFormat {
    R8 = 0,       // 1 channel, 8-bit
    RG8 = 1,      // 2 channels, 8-bit
    RGB8 = 2,     // 3 channels, 8-bit
    RGBA8 = 3,    // 4 channels, 8-bit

    R16 = 0,       // 1 channel, 16-bit
    RG16 = 1,      // 2 channels, 16-bit
    RGB16 = 2,     // 3 channels, 16-bit
    RGBA16 = 3,    // 4 channels, 16-bit

    // Below is definition of EXR/HDR formats. High dynamic-range colors often used for textures such as BRDF_LUT in which color precision matters for later calculations
    R32F = 8,     // 1 channel, 32-bit float
    RG32F = 9,    // 2 channels, 32-bit float
    RGB32F = 10,  // 3 channels, 32-bit float
    RGBA32F = 11  // 4 channels, 32-bit float
};

const ImageFormatInfo& getImageFormatInfo(ImageFormat fmt);
ImageFormat getImageFormat(int nrChannels, int bytesPerChannel, bool isHDR);
GLint getUnpackAlignment(int width, int channels, int bytesPerChannel);
GLint getUnpackAlignment(int width, ImageFormatInfo info);

void* loadSTBI_File(const std::filesystem::path& path, ImageFormat& fmt_out, int& width_out, int& height_out, int& nrChannels_out);;
void* loadSTBI_FileBytes(const uint8_t* bytes, int length, ImageFormat& fmt_out, int& width_out, int& height_out, int& nrChannels_out);

struct TextureImage {
    ImageFormat format = ImageFormat::RGB8;
    std::vector<uint8_t> bytes;
    int width, height;

    const ImageFormatInfo& getInfo() const {
        return getImageFormatInfo(format);
    }
    size_t getSize() const {
        const ImageFormatInfo& info = getInfo();
        return width * height * info.nrComponents * info.bits();
    }
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
    const TextureImage& getImage() const { return image; }

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
    const std::array<TextureImage, 6>& getImages() const { return images; }

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

std::future<GLuint> uploadTex2DFromBytes_TQ(std::vector<uint8_t> pixels, int width, int height, ImageFormatInfo info, TexelExecParams exec_params);
GLuint uploadTex2DFromBytes(void *raw, int width, int height, ImageFormatInfo info, TexelExecParams exec_params);
GLuint uploadTex2DFromFileBytes(uint8_t *bytes, int length, TexelExecParams exec_params);
GLuint uploadTex2DFromFile(const std::filesystem::path& path, TexelExecParams exec_params);

std::future<GLuint> uploadTexCubeFromBytes_TQ(std::vector<std::vector<uint8_t>> pixels, int *widths, int *heights, ImageFormatInfo *infos, TexelExecParams exec_params);
GLuint uploadTexCubeFromBytes(void **raw, int widths[6], int heights[6], ImageFormatInfo infos[6], TexelExecParams exec_params);
GLuint uploadTexCubeFromFilesBytes(uint8_t **bytes, int lengths[6], TexelExecParams exec_params);
GLuint uploadTexCubeFromFiles(std::vector<std::filesystem::path>& paths, TexelExecParams exec_params);

}