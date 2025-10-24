#include "Texture.hpp"

#include "Syngine/Syngine.hpp"
#include "Syngine/engine/TaskQueue.hpp"

#include "Syngine/modules/Model.hpp"
#include "Syngine/modules/Presets.hpp"

#include "Syngine/serialization/DataSerializer.hpp"
#include "Syngine/serialization/DataTemplates.hpp"

#include <cstdint>
#include <fstream>
#include <future>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace syng;

std::vector<uint8_t> readTextureBytes(std::string path) {
    std::ifstream textureFile;

    textureFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    textureFile.open(path, std::ios::binary);
    std::vector<uint8_t> texel_bytes((std::istreambuf_iterator<char>(textureFile)), {});
    textureFile.close();

    return texel_bytes;
}

TextureWriter::TextureWriter(DataSerializer *buff) : buffer(buff) {}

void TextureWriter::writeTexture2D(std::string path, std::vector<uint8_t> bytes) {
    DataTemplates::write_uint16(buffer, PCK_HEADER_TEX2D);
    DataTemplates::write_string(buffer, path);
    DataTemplates::write_uint64(buffer, bytes.size());
    buffer->write(bytes.data(), bytes.size());
    DataTemplates::write_uint16(buffer, PCK_FOOTER_TEX2D);
}

void TextureWriter::writeTexture2D(Texture2D tex2D) {
    DataTemplates::write_uint16(buffer, PCK_HEADER_TEX2D);
    DataTemplates::write_string(buffer, tex2D.path);

    std::vector<uint8_t> texel_bytes = readTextureBytes(tex2D.path);

    DataTemplates::write_uint64(buffer, texel_bytes.size());
    buffer->write(texel_bytes.data(), texel_bytes.size());
    DataTemplates::write_uint16(buffer, PCK_FOOTER_TEX2D);
}

void TextureWriter::writeTextureCubemap(TextureCubemap texQB) {
    DataTemplates::write_uint16(buffer, PCK_HEADER_TEXQBMP);
    for (int i = 0; i < 6; i++) {
        std::vector<uint8_t> bytes = readTextureBytes(texQB.paths[i]);

        DataTemplates::write_string(buffer, texQB.paths[i]);
        DataTemplates::write_int32(buffer, bytes.size());
        buffer->write(bytes.data(), bytes.size());
    }
    DataTemplates::write_uint16(buffer, PCK_FOOTER_TEXQBMP);
}

void TextureWriter::writeTextureCubemap(std::string paths[6], std::vector<uint8_t> bytes[6]) {
    DataTemplates::write_uint16(buffer, PCK_HEADER_TEXQBMP);
    for (int i = 0; i < 6; i++) {
        DataTemplates::write_string(buffer, paths[i]);
        DataTemplates::write_int32(buffer, bytes[i].size());
        buffer->write(bytes[i].data(), bytes[i].size());
    }
    DataTemplates::write_uint16(buffer, PCK_FOOTER_TEXQBMP);
}

void TextureWriter::writeMeshTexture2D(MaterialTexture2D_T texType, Texture2D texel) {
    DataTemplates::write_uint16(buffer, PCK_HEADER_TEX2D_MESH);
    TextureWriter::writeTexture2D(texel);
    DataTemplates::write_int32(buffer, texType);
    DataTemplates::write_uint16(buffer, PCK_FOOTER_TEX2D_MESH);
}

void TextureWriter::writeMeshTexture2D(std::string path, std::vector<uint8_t> bytes, MaterialTexture2D_T type) {
    DataTemplates::write_uint16(buffer, PCK_HEADER_TEX2D_MESH);
    TextureWriter::writeTexture2D(path, bytes);
    DataTemplates::write_int32(buffer, type);
    DataTemplates::write_uint16(buffer, PCK_FOOTER_TEX2D_MESH);
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

TextureCubemap syng::loadTextureCubemap(std::vector<std::filesystem::path> paths) {
    if (paths.size() != 6) throw std::runtime_error("loadTextureCubemap(): paths.size != 6");
    TextureCubemap tex;
    for (int i = 0; i < 6; i++) {
        tex.paths[i] = paths[i].string();
    }
    tex.TCB = QBTCBFromFiles(paths);
    return tex;
}

TextureCubemap syng::loadTextureCubemap(DataDeserializer *buffer) {
    DataTemplates::push(buffer, "TextureCubemap", PCK_HEADER_TEXQBMP);

    TextureCubemap tex;
    uint8_t *bytes[6];
    int lengths[6];

    for (int i = 0; i < 6; i++) {
        tex.paths[i] = DataTemplates::read_string(buffer);
        lengths[i] = DataTemplates::read_int32(buffer);
        buffer->read(bytes[i], lengths[i]);
    }
    tex.TCB = QBTCBFromFilesBytes(bytes, lengths);

    DataTemplates::pop(buffer, "TextureCubemap", PCK_FOOTER_TEXQBMP);
    return tex;
}

std::pair<MaterialTexture2D_T, Texture2D> syng::loadMeshTexture2D(DataDeserializer *buffer) {
    DataTemplates::push(buffer, "MeshTexture2D", PCK_HEADER_TEX2D_MESH);
    Texture2D texel = loadTexture2D(buffer);
    MaterialTexture2D_T texType = static_cast<MaterialTexture2D_T>(DataTemplates::read_int32(buffer));
    DataTemplates::pop(buffer, "MeshTexture2D", PCK_FOOTER_TEX2D_MESH);
    return {texType, texel};
}

Texture2D syng::loadTexture2D(const std::filesystem::path& path) {
    Texture2D texture;
    texture.path = path.string();
    texture.TCB = TCBFromFile(path);
    return texture;
}

Texture2D syng::loadTexture2D(DataDeserializer *buffer) {
    DataTemplates::push(buffer, "Texture2D", PCK_HEADER_TEX2D);
    
    Texture2D texture;
    texture.path = DataTemplates::read_string(buffer);
    
    uint64_t size = DataTemplates::read_uint64(buffer);
    std::vector<uint8_t> img(size);
    buffer->read(img.data(), size);
    texture.TCB = TCBFromFileBytes(img.data(), size);

    DataTemplates::pop(buffer, "Texture2D", PCK_FOOTER_TEX2D);
    return texture;
}

GLuint syng::TCBFromBytes(uint8_t *raw, int width, int height, int nrComponents) {
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

#ifdef __EMSCRIPTEN__
    // if (format == GL_RGBA || format == GL_RGB || format == GL_SRGB8_ALPHA8) {
    //     glGenerateMipmap(GL_TEXTURE_2D);
    //     PresetsTexel::TextureParamST(GL_TEXTURE_2D, GL_REPEAT);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // } else {
        PresetsTexel::TextureParamST(GL_TEXTURE_2D, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // }
#else
    glGenerateMipmap(GL_TEXTURE_2D);

    PresetsTexel::TextureParamST(GL_TEXTURE_2D, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
    return TCB;
}

std::future<GLuint> syng::TCBFromBytes_TQ(uint8_t *raw, int width, int height, int nrComponents) {
    return TaskQueue::Instance().enqueueFuture([&](){
        return TCBFromBytes(raw, width, height, nrComponents);
    });
}

GLuint syng::TCBFromFileBytes(uint8_t *bytes, int length) {
    int width, height, nrComponents;
    uint8_t *data = stbi_load_from_memory(bytes, length, &width, &height, &nrComponents, 0);

    if (!data) {
        throw std::runtime_error("TCBFromFileBytes(): Couldn't Load Image data in bytes, probably one or more textures are corrupted.");
    }
    GLuint TCB = TCBFromBytes_TQ(data, width, height, nrComponents).get();
    stbi_image_free(data);
    return TCB;
}

GLuint syng::TCBFromFile(const std::filesystem::path& path) {
    int width, height, nrComponents;
    uint8_t *data = stbi_load(path.string().c_str(), &width, &height, &nrComponents, 0);

    if (data) {
        GLuint TCB = TCBFromBytes_TQ(data, width, height, nrComponents).get();
        stbi_image_free(data);
        return TCB;
    } else {
        std::cerr << "TCBFromFile(): Couldn't Load Image data at path: " << path << std::endl;
        stbi_image_free(data);
        return 0;
    }
}

GLuint syng::QBTCBFromBytes(uint8_t **raws, int *widths, int *heights, int *nrComponentss) {
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
    PresetsTexel::TextureFilter(GL_TEXTURE_CUBE_MAP);
    PresetsTexel::TextureParamSTR(GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE);

    return QBTCB;
}

std::future<GLuint> syng::QBTCBFromBytes_TQ(uint8_t **raws, int *widths, int *heights, int *nrComponentss) {
    return TaskQueue::Instance().enqueueFuture([&](){
        return QBTCBFromBytes(raws, widths, heights, nrComponentss);
    });
}

GLuint syng::QBTCBFromFilesBytes(unsigned char **bytes, int *lengths) {
    uint8_t *raws[6];
    int widths[6], heights[6], nrComponents[6];

    for (unsigned int i = 0; i < 6; i++) {
        raws[i] = stbi_load_from_memory(bytes[i], lengths[i], &widths[i], &heights[i], &nrComponents[i], 0);    
        if (!raws[i]) {
            throw std::runtime_error("QBTCBFromFilesBytes(): Couldn't Load Image data in bytes, probably one or more textures are corrupted.");
        }
    }
    GLuint QBTCB = QBTCBFromBytes_TQ(raws, widths, heights, nrComponents).get();
    for (unsigned int i = 0; i < 6; i++) stbi_image_free(raws[i]);
    return QBTCB;
}

GLuint syng::QBTCBFromFiles(std::vector<std::filesystem::path>& paths) {
    if (paths.size() != 6) throw std::runtime_error("Cubemap paths size != 6");

    uint8_t *raws[6];
    int widths[6], heights[6], nrComponents[6];

    for (unsigned int i = 0; i < 6; i++) {
        raws[i] = stbi_load(paths[i].string().c_str(), &widths[i], &heights[i], &nrComponents[i], 0);    
        if (!raws[i]) {
            throw std::runtime_error("QBTCBFromFilesBytes(): Couldn't Load Image data in bytes, probably one or more textures are corrupted.");
        }
    }
    GLuint QBTCB = QBTCBFromBytes_TQ(raws, widths, heights, nrComponents).get();
    for (unsigned int i = 0; i < 6; i++) stbi_image_free(raws[i]);
    return QBTCB;
}