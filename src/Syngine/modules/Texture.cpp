#include "Texture.hpp"

#include "Syngine/Syngine.hpp"
#include "Syngine/engine/TaskQueue.hpp"

#include "Syngine/modules/Model.hpp"
#include "Syngine/modules/Presets.hpp"

#include "Syngine/modules/Texture.hpp"
#include "Syngine/ports/GLPort.h"
#include "Syngine/serialization/DataSerializer.hpp"
#include "Syngine/serialization/DataTemplates.hpp"

#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace syng;

Texture2D::Texture2D(GLuint TCB) : TCB(TCB) {}
Texture2D::Texture2D(TextureImage image, std::string path) : image(image), path(path) {}

void Texture2D::setTCB(GLuint nTCB) {
    deleteTexture();
    this->TCB = nTCB;
}
GLuint Texture2D::getTCB() const { return TCB; }
const std::string& Texture2D::getPath() const { return path; }

void Texture2D::readImage() { image = readTextureImage(path); }
void Texture2D::clearImage() { image = {}; }

bool Texture2D::isUploaded() { return TCB != 0; }

void Texture2D::deleteTexture() { if (TCB) glDeleteTextures(1, &TCB); }
void Texture2D::uploadTexture(std::function<void(GLuint TCB)> exec_params) {
    setTCB(uploadTex2DFromBytes(image.bytes.data(), image.width, image.height, image.nrComponents, exec_params));
}

TextureCubemap::TextureCubemap(GLuint TCB) : TCB(TCB) {}
TextureCubemap::TextureCubemap(std::array<TextureImage, 6> images, std::array<std::string, 6> paths) : images(images), paths(paths) {}
TextureCubemap::~TextureCubemap() {
    if (TCB) glDeleteTextures(1, &TCB);
    clearImages();
}

void TextureCubemap::setTCB(GLuint TCB) { 
    deleteTexture();
    this->TCB = TCB;
}
GLuint TextureCubemap::getTCB() const { return TCB; }
const std::string& TextureCubemap::getPath(GLenum face) const { 
    static std::string empty_path = "";
    int idx = face - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
    return idx < 0 || idx > 5 ? empty_path : paths[idx];
}

void TextureCubemap::readImages() { for (int i = 0; i < 6; i++) images[i] = readTextureImage(paths[i]); }
void TextureCubemap::clearImages() { for (int i = 0; i < 6; i++) images[i] = {}; }

bool TextureCubemap::isUploaded() { return TCB != 0; }

void TextureCubemap::deleteTexture() { if (TCB) glDeleteTextures(1, &TCB); }
void TextureCubemap::uploadTexture(std::function<void(GLuint TCB)> exec_params) {
    uint8_t *raws[6];
    int widths[6], heights[6], nrComponentss[6];
    for (unsigned int i = 0; i < 6; i++) {
        raws[i] = images[i].bytes.data();
        widths[i] = images[i].width;
        heights[i] = images[i].height;
        nrComponentss[i] = images[i].nrComponents;
    }
    setTCB(uploadTexCubeFromBytes(raws, widths, heights, nrComponentss, exec_params));
}

TextureImage syng::readTextureImage(const std::filesystem::path& path) {
    std::ifstream textureFile;

    textureFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    textureFile.open(path, std::ios::binary);
    std::vector<uint8_t> file_bytes((std::istreambuf_iterator<char>(textureFile)), {});
    textureFile.close();

    int width, height, nrComponents;
    uint8_t *data = stbi_load_from_memory(file_bytes.data(), file_bytes.size(), &width, &height, &nrComponents, 0);

    std::vector<uint8_t> texel_bytes(data, data + static_cast<size_t>(width) * height * nrComponents);
    stbi_image_free(data);
    
    return {texel_bytes, width, height, nrComponents};
}

TextureIO::TexturePackedWriter::TexturePackedWriter(DataSerializer *buff) : buffer(buff) {}

void TextureIO::TexturePackedWriter::writeTexture2D(std::string path, TextureImage& image) {
    DataTemplates::write_uint16(buffer, PCK_HEADER_TEX2D);
    DataTemplates::write_string(buffer, path);
    DataTemplates::write_texture_image(buffer, image);
    DataTemplates::write_uint16(buffer, PCK_FOOTER_TEX2D);
}

void TextureIO::TexturePackedWriter::writeTexture2D(Texture2D tex2D) {
    TextureImage image = readTextureImage(tex2D.getPath());
    writeTexture2D(tex2D.getPath(), image);
}

void TextureIO::TexturePackedWriter::writeTextureCubemap(std::string paths[6], TextureImage images[6]) {
    DataTemplates::write_uint16(buffer, PCK_HEADER_TEXQBMP);
    for (int i = 0; i < 6; i++) {
        DataTemplates::write_string(buffer, paths[i]);
        DataTemplates::write_texture_image(buffer, images[i]);
    }
    DataTemplates::write_uint16(buffer, PCK_FOOTER_TEXQBMP);
}

void TextureIO::TexturePackedWriter::writeTextureCubemap(TextureCubemap texQB) {
    DataTemplates::write_uint16(buffer, PCK_HEADER_TEXQBMP);
    for (int i = 0; i < 6; i++) {
        TextureImage image = readTextureImage(texQB.getPath(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i));
        DataTemplates::write_texture_image(buffer, image);
    }
    DataTemplates::write_uint16(buffer, PCK_FOOTER_TEXQBMP);
}

void TextureIO::TexturePackedWriter::writeMeshTexture2D(std::string path, TextureImage& image, MaterialTexture2D_T type) {
    DataTemplates::write_uint16(buffer, PCK_HEADER_TEX2D_MESH);
    writeTexture2D(path, image);
    DataTemplates::write_int32(buffer, type);
    DataTemplates::write_uint16(buffer, PCK_FOOTER_TEX2D_MESH);
}

void TextureIO::TexturePackedWriter::writeMeshTexture2D(MaterialTexture2D_T texType, Texture2D texel) {
    TextureImage image = readTextureImage(texel.getPath());
    writeMeshTexture2D(texel.getPath(), image, texType);
}

void syng::TCBPlainColor(unsigned int &TCB, unsigned char pixel[4]) {
    glGenTextures(1, &TCB);
    glBindTexture(GL_TEXTURE_2D, TCB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    PresetsTexel::TextureFilter(GL_TEXTURE_2D, GL_NEAREST);
    PresetsTexel::TextureParamST(GL_TEXTURE_2D, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint syng::TCBByPlainColor(unsigned char pixel[4]) {
    GLuint TCB;
    TCBPlainColor(TCB, pixel);
    return TCB;
}

TextureCubemap TextureIO::TextureFileReader::readTextureCubemap(std::vector<std::filesystem::path> paths) {
    if (paths.size() != 6) throw std::runtime_error("loadTextureCubemap(): paths.size != 6");
    std::array<std::string, 6> paths_str;
    for (int i = 0; i < 6; i++) {
        paths_str[i] = paths[i].string();
    }
    std::array<TextureImage, 6> images;
    for (int i = 0; i < 6; i++) {
        images[i] = readTextureImage(paths_str[i]);
    }
    return TextureCubemap(images, paths_str);
}

TextureIO::TexturePackedReader::TexturePackedReader(DataDeserializer *buffer) : buffer(buffer) {}

TextureCubemap TextureIO::TexturePackedReader::readTextureCubemap() {
    DataTemplates::push(buffer, "TextureCubemap", PCK_HEADER_TEXQBMP);

    std::array<std::string, 6> paths;
    std::array<TextureImage, 6> images;

    for (int i = 0; i < 6; i++) {
        paths[i] = DataTemplates::read_string(buffer);
        images[i] = DataTemplates::read_texture_image(buffer);
    }
    DataTemplates::pop(buffer, "TextureCubemap", PCK_FOOTER_TEXQBMP);
    return {images, paths};
}

std::pair<MaterialTexture2D_T, Texture2D> TextureIO::TexturePackedReader::readMeshTexture2D() {
    DataTemplates::push(buffer, "MeshTexture2D", PCK_HEADER_TEX2D_MESH);
    Texture2D texel = readTexture2D();
    MaterialTexture2D_T texType = static_cast<MaterialTexture2D_T>(DataTemplates::read_int32(buffer));
    DataTemplates::pop(buffer, "MeshTexture2D", PCK_FOOTER_TEX2D_MESH);
    return {texType, texel};
}

Texture2D TextureIO::TextureFileReader::readTexture2D(const std::filesystem::path& path) {
    return {readTextureImage(path), path.string()};
}

Texture2D TextureIO::TexturePackedReader::readTexture2D() {
    DataTemplates::push(buffer, "Texture2D", PCK_HEADER_TEX2D);
    std::string path = DataTemplates::read_string(buffer);
    TextureImage image = DataTemplates::read_texture_image(buffer);
    DataTemplates::pop(buffer, "Texture2D", PCK_FOOTER_TEX2D);
    return {image, path};
}

GLuint syng::uploadTex2DFromBytes(uint8_t *raw, int width, int height, int nrComponents, TexelExecParams exec_params) {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLuint TCB;
    GLenum format;

    switch (nrComponents) {
        case 1: format = GL_RED; break;
        case 3: format = GL_RGB; break;
        case 4: format = GL_RGBA; break;
        default: return 0;
    }
#ifndef __EMSCRIPTEN__
    GLenum internalFormat = (nrComponents == 3) ? GL_RGB8 : (nrComponents == 4) ? GL_RGBA8 : GL_R8;
#else
    GLenum internalFormat = (nrComponents == 3) ? GL_RGB : (nrComponents == 4) ? GL_RGBA : GL_RED;
#endif
    glGenTextures(1, &TCB);
    glBindTexture(GL_TEXTURE_2D, TCB);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, raw);
    exec_params(TCB);
    return TCB;
}

std::future<GLuint> syng::uploadTex2DFromBytes_TQ(uint8_t *raw, int width, int height, int nrComponents, TexelExecParams exec_params) {
    return TaskQueue::Instance().enqueueFuture([&](){
        GLuint TCB = uploadTex2DFromBytes(raw, width, height, nrComponents, exec_params);
        exec_params(TCB);
        return TCB;
    });
}

GLuint syng::uploadTex2DFromFileBytes(uint8_t *bytes, int length, TexelExecParams exec_params) {
    int width, height, nrComponents;
    uint8_t *data = stbi_load_from_memory(bytes, length, &width, &height, &nrComponents, 0);

    if (!data) {
        throw std::runtime_error("TCBFromFileBytes(): Couldn't Load Image data in bytes, probably one or more textures are corrupted.");
    }
    GLuint TCB = uploadTex2DFromBytes_TQ(data, width, height, nrComponents, exec_params).get();
    stbi_image_free(data);
    return TCB;
}

GLuint syng::uploadTex2DFromFile(const std::filesystem::path& path, TexelExecParams exec_params) {
    int width, height, nrComponents;
    uint8_t *data = stbi_load(path.string().c_str(), &width, &height, &nrComponents, 0);

    if (data) {
        GLuint TCB = uploadTex2DFromBytes_TQ(data, width, height, nrComponents, exec_params).get();
        stbi_image_free(data);
        return TCB;
    } else {
        std::cerr << "TCBFromFile(): Couldn't Load Image data at path: " << path << std::endl;
        stbi_image_free(data);
        return 0;
    }
}

GLuint syng::uploadTexCubeFromBytes(uint8_t **raws, int *widths, int *heights, int *nrComponentss, TexelExecParams exec_params) {
    GLuint QBTCB;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &QBTCB);
    glBindTexture(GL_TEXTURE_CUBE_MAP, QBTCB);

    for (unsigned int i = 0; i < 6; i++) {
        uint8_t *raw = raws[i];
        int width = widths[i], height = heights[i], nrComponents = nrComponentss[i];

        GLenum format;
        switch (nrComponents) {
            case 1: format = GL_RED; break;
            case 3: format = GL_RGB; break;
            case 4: format = GL_RGBA; break;
            default: return 0;
        }
        if (raw) {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format,
                width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, raw
            );
        }
    }
    exec_params(QBTCB);
    return QBTCB;
}

std::future<GLuint> syng::uploadTexCubeFromBytes_TQ(uint8_t **raws, int *widths, int *heights, int *nrComponentss, TexelExecParams exec_params) {
    return TaskQueue::Instance().enqueueFuture([&](){
        GLuint TCB = uploadTexCubeFromBytes(raws, widths, heights, nrComponentss, exec_params);
        exec_params(TCB);
        return TCB;
    });
}

GLuint syng::uploadTexCubeFromFilesBytes(unsigned char **bytes, int *lengths, TexelExecParams exec_params) {
    uint8_t *raws[6];
    int widths[6], heights[6], nrComponents[6];

    for (unsigned int i = 0; i < 6; i++) {
        raws[i] = stbi_load_from_memory(bytes[i], lengths[i], &widths[i], &heights[i], &nrComponents[i], 0);    
        if (!raws[i]) {
            throw std::runtime_error("QBTCBFromFilesBytes(): Couldn't Load Image data in bytes, probably one or more textures are corrupted.");
        }
    }
    GLuint QBTCB = uploadTexCubeFromBytes_TQ(raws, widths, heights, nrComponents, exec_params).get();
    for (unsigned int i = 0; i < 6; i++) stbi_image_free(raws[i]);
    return QBTCB;
}

GLuint syng::uploadTexCubeFromFiles(std::vector<std::filesystem::path>& paths, TexelExecParams exec_params) {
    if (paths.size() != 6) throw std::runtime_error("Cubemap paths size != 6");

    uint8_t *raws[6];
    int widths[6], heights[6], nrComponents[6];

    for (unsigned int i = 0; i < 6; i++) {
        raws[i] = stbi_load(paths[i].string().c_str(), &widths[i], &heights[i], &nrComponents[i], 0);    
        if (!raws[i]) {
            throw std::runtime_error("QBTCBFromFilesBytes(): Couldn't Load Image data in bytes, probably one or more textures are corrupted.");
        }
    }
    GLuint QBTCB = uploadTexCubeFromBytes_TQ(raws, widths, heights, nrComponents, exec_params).get();
    for (unsigned int i = 0; i < 6; i++) stbi_image_free(raws[i]);
    return QBTCB;
}