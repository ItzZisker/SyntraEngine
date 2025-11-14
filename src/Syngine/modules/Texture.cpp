#include "Texture.hpp"

#include "Presets.hpp"

#include "Syngine/engine/TaskQueue.hpp"
#include "Syngine/serialization/DataSerializer.hpp"
#include "Syngine/serialization/DataTemplates.hpp"
#include "Syngine/ports/GLPort.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "stb_image.h"

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
    setTCB(uploadTex2DFromBytes((void*)image.bytes.data(), image.bytes.size(), image.width, image.height, image.getInfo(), exec_params));
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
    void *raws[6];
    int widths[6], heights[6], lengths[6];
    ImageFormatInfo infos[6];
    for (unsigned int i = 0; i < 6; i++) {
        raws[i] = (void*)images[i].bytes.data();
        lengths[i] = images[i].bytes.size();
        widths[i] = images[i].width;
        heights[i] = images[i].height;
        infos[i] = images[i].getInfo();
    }
    setTCB(uploadTexCubeFromBytes(raws, lengths, widths, heights, infos, exec_params));
}

const ImageFormatInfo& syng::getImageFormatInfo(ImageFormat fmt) {
    static std::array<ImageFormatInfo, 18> ImageFormatInfoMap = {{
        {GL_R8,    GL_RED,  GL_UNSIGNED_BYTE, 1},
        {GL_RG8,   GL_RG,   GL_UNSIGNED_BYTE, 2},
        {GL_RGB8,  GL_RGB,  GL_UNSIGNED_BYTE, 3},
        {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 4},

        {GL_R16,    GL_RED,  GL_UNSIGNED_BYTE, 1},
        {GL_RG16,   GL_RG,   GL_UNSIGNED_BYTE, 2},
        {GL_RGB16,  GL_RGB,  GL_UNSIGNED_BYTE, 3},
        {GL_RGBA16, GL_RGBA, GL_UNSIGNED_BYTE, 4},

        {GL_R32F,    GL_RED,  GL_FLOAT, 1},
        {GL_RG32F,   GL_RG,   GL_FLOAT, 2},
        {GL_RGB32F,  GL_RGB,  GL_FLOAT, 3},
        {GL_RGBA32F, GL_RGBA, GL_FLOAT, 4},

        {GL_RGBA8, GL_COMPRESSED_RGBA_ASTC_4x4, GL_UNSIGNED_BYTE, 4},
        {GL_RGBA8, GL_COMPRESSED_RGBA_ASTC_5x5, GL_UNSIGNED_BYTE, 4},
        {GL_RGBA8, GL_COMPRESSED_RGBA_ASTC_6x6, GL_UNSIGNED_BYTE, 4},
        {GL_RGBA8, GL_COMPRESSED_RGBA_ASTC_8x8, GL_UNSIGNED_BYTE, 4},

        {GL_RGB8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_UNSIGNED_BYTE, 3},
        {GL_RGBA8, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_UNSIGNED_BYTE, 4},
    }};
    return ImageFormatInfoMap[static_cast<int>(fmt)];
}

ImageFormat syng::getImageFormat(int nrChannels, int bits, bool isHDR) {
    if (isHDR && bits == 32) {
        switch (nrChannels) {
            case 1: return ImageFormat::R32F;
            case 2: return ImageFormat::RG32F;
            case 3: return ImageFormat::RGB32F;
            case 4: return ImageFormat::RGBA32F;
        }
    } else {
        if (bits == 8) {
            switch (nrChannels) {
                case 1: return ImageFormat::R8;
                case 2: return ImageFormat::RG8;
                case 3: return ImageFormat::RGB8;
                case 4: return ImageFormat::RGBA8;
            }
        } else if (bits == 16) {
            switch (nrChannels) {
                case 1: return ImageFormat::R16;
                case 2: return ImageFormat::RG16;
                case 3: return ImageFormat::RGB16;
                case 4: return ImageFormat::RGBA16;
            }
        }
    }
    throw std::runtime_error("ImageFormat not found for " + std::to_string(nrChannels) + ", isHDR=" + std::to_string(isHDR));
}

GLint syng::getUnpackAlignment(int width, int channels, int bytesPerChannel) {
    int rowStride = width * channels * bytesPerChannel;
    if (rowStride % 8 == 0) return 8;
    if (rowStride % 4 == 0) return 4;
    if (rowStride % 2 == 0) return 2;
    return 1;
}

GLint syng::getUnpackAlignment(int width, ImageFormatInfo info) {
    return getUnpackAlignment(width, info.nrComponents, info.bits());
}

ASTCImage syng::loadASTC_FileBytes(DataDeserializer *buffer) {
    typedef struct {
        uint8_t magic[4];       // 0x13, 0xAB, 0xA1, 0x5C
        uint8_t blockdim_x;     // usually 4–12
        uint8_t blockdim_y;     // usually 4–12
        uint8_t blockdim_z;     // usually 1
        uint8_t xsize[3];       // little-endian 24-bit
        uint8_t ysize[3];
        uint8_t zsize[3];
    } ASTCHeader;

    ASTCImage img{};
    ASTCHeader header;
    buffer->read(reinterpret_cast<uint8_t*>(&header), sizeof(header));

    if (header.magic[0] != 0x13 || header.magic[1] != 0xAB || header.magic[2] != 0xA1 || header.magic[3] != 0x5C) {
        throw std::runtime_error("Not a valid ASTC file");
    }

    img.blockX = header.blockdim_x;
    img.blockY = header.blockdim_y;
    img.blockZ = header.blockdim_z;

    img.width  = header.xsize[0] + (header.xsize[1] << 8) + (header.xsize[2] << 16);
    img.height = header.ysize[0] + (header.ysize[1] << 8) + (header.ysize[2] << 16);
    img.depth  = header.zsize[0] + (header.zsize[1] << 8) + (header.zsize[2] << 16);

    // Read compressed data
    size_t dataSize = buffer->getLength() - sizeof(ASTCHeader);
    img.data.resize(dataSize);
    buffer->read(img.data.data(), dataSize);

    return img;
}

ASTCImage syng::loadASTC_FileBytes(const uint8_t* bytes, int length) {
    std::shared_ptr<BufferDataStream> stream = std::make_shared<BufferDataStream>(bytes, length);
    DataDeserializer *buffer = new DataDeserializer(stream);
    ASTCImage img = loadASTC_FileBytes(buffer);
    delete buffer;
    return img;
}

ASTCImage syng::loadASTC_File(const std::filesystem::path& path) {
    std::shared_ptr<FileDataStream> stream = std::make_shared<FileDataStream>(path, true, false);
    DataDeserializer *buffer = new DataDeserializer(stream);
    ASTCImage img = loadASTC_FileBytes(buffer);
    delete buffer;
    return img;
}

bool syng::isASTC_FileBytes(DataDeserializer *buffer) {
    uint8_t magic[4];
    buffer->read(magic, 4);
    return magic[0] == 0x13 && magic[1] == 0xAB && magic[2] == 0xA1 && magic[3] == 0x5C;
}

bool syng::isASTC_FileBytes(const uint8_t* bytes, int length) {
    if (length < 4 + 20) return false;
    std::shared_ptr<BufferDataStream> stream = std::make_shared<BufferDataStream>(bytes, length);
    DataDeserializer *buffer = new DataDeserializer(stream);
    bool result = isASTC_FileBytes(buffer);
    delete buffer;
    return result;
}

bool syng::isASTC_File(const std::filesystem::path& path) {
    std::shared_ptr<FileDataStream> stream = std::make_shared<FileDataStream>(path, true, false);
    DataDeserializer *buffer = new DataDeserializer(stream);
    if (buffer->getLength() < 4 + 20) return false;
    bool result = isASTC_FileBytes(buffer);
    delete buffer;
    return result;
}

DXTImage syng::loadDXT_FileBytes(DataDeserializer *buffer) {
    typedef struct {
        uint8_t magic[4];   // "DXT\0"
        uint16_t width;
        uint16_t height;
        uint8_t alpha;      // 0 = BC1/DXT1, 1 = BC3/DXT5
        uint8_t reserved;   // alignment padding
    } DXTHeader;

    DXTImage img{};
    DXTHeader header;
    buffer->read(reinterpret_cast<uint8_t*>(&header), sizeof(header));

    if (header.magic[0] != 'D' || header.magic[1] != 'X' || header.magic[2] != 'T' || header.magic[3] != '\0') {
        throw std::runtime_error("Not a valid DXT file");
    }

    img.width = header.width;
    img.height = header.height;
    img.alpha = header.alpha;

    // Read compressed data
    size_t dataSize = buffer->getLength() - sizeof(DXTHeader);
    img.data.resize(dataSize);
    buffer->read(img.data.data(), dataSize);

    return img;
}

DXTImage syng::loadDXT_FileBytes(const uint8_t* bytes, int length) {
    std::shared_ptr<BufferDataStream> stream = std::make_shared<BufferDataStream>(bytes, length);
    DataDeserializer *buffer = new DataDeserializer(stream);
    DXTImage img = loadDXT_FileBytes(buffer);
    delete buffer;
    return img;
}

DXTImage syng::loadDXT_File(const std::filesystem::path& path) {
    std::shared_ptr<FileDataStream> stream = std::make_shared<FileDataStream>(path, true, false);
    DataDeserializer *buffer = new DataDeserializer(stream);
    DXTImage img = loadDXT_FileBytes(buffer);
    delete buffer;
    return img;
}

bool syng::isDXT_FileBytes(DataDeserializer *buffer) {
    uint8_t magic[4];
    buffer->read(magic, 4);
    return magic[0] == 'D' && magic[1] == 'X' && magic[2] == 'T' && magic[3] == '\0';
}

bool syng::isDXT_FileBytes(const uint8_t* bytes, int length) {
    if (length < 4 + 4) return false;
    std::shared_ptr<BufferDataStream> stream = std::make_shared<BufferDataStream>(bytes, length);
    DataDeserializer *buffer = new DataDeserializer(stream, 4);
    bool result = isDXT_FileBytes(buffer);
    delete buffer;
    return result;
}

bool syng::isDXT_File(const std::filesystem::path& path) {
    std::shared_ptr<FileDataStream> stream = std::make_shared<FileDataStream>(path, true, false);
    DataDeserializer *buffer = new DataDeserializer(stream, 8);
    if (buffer->getLength() < 4 + 4) return false;
    bool result = isDXT_FileBytes(buffer);
    delete buffer;
    return result;
}

void* syng::loadRawImage_File(const std::filesystem::path& path, ImageFormat& format, int& width, int& height, int& channels) {
    std::string path_str = path.string();
    const char *path_cstr = path_str.c_str();
    
    void *data = nullptr;
    if (isDXT_File(path)) {
        auto dxt = loadDXT_File(path);
        width = dxt.width;
        height = dxt.height;
        channels = dxt.alpha ? 4 : 3;
        format = dxt.alpha ? ImageFormat::RGBA8_BC3 : ImageFormat::RGB8_BC1;
        data = malloc(dxt.data.size());
        std::memcpy(data, dxt.data.data(), dxt.data.size());
    } else if (isASTC_File(path)) {
        auto astc = loadASTC_File(path);
        if (astc.blockX == 4 && astc.blockY == 4) format = ImageFormat::RGBA8_ASTC4x4;
        if (astc.blockX == 5 && astc.blockY == 5) format = ImageFormat::RGBA8_ASTC5x5;
        if (astc.blockX == 6 && astc.blockY == 6) format = ImageFormat::RGBA8_ASTC6x6;
        if (astc.blockX == 8 && astc.blockY == 8) format = ImageFormat::RGBA8_ASTC8x8;
        width = astc.width;
        height = astc.height;
        channels = 4;
        data = malloc(astc.data.size());
        std::memcpy(data, astc.data.data(), astc.data.size());
    } else if (stbi_is_hdr(path_cstr)) {
        data = stbi_loadf(path_cstr, &width, &height, &channels, 0);
        if (!data) {
            throw std::runtime_error("Failed to load image 'F32' from path=" + path_str);
        }
        format = getImageFormat(channels, 32, true);
    } else {
        // Idk gives weird texture artifacts + How the hell am I supposed to know whether image is 16/8 bits ???
        //data = stbi_load_16(path_cstr, &width, &height, &channels, 0);
        if (!data) {
            data = stbi_load(path_cstr, &width, &height, &channels, 0);
            if (!data) {
                throw std::runtime_error("Failed to load image 'U8/16' from path=" + path_str);
            }
            format = getImageFormat(channels, 8, false);
        } else {
            format = getImageFormat(channels, 16, false);
        }
    }
    return data;
}

void* syng::loadRawImage_FileBytes(const uint8_t* bytes, int length, ImageFormat& format, int& width, int& height, int& channels) {
    void *data = nullptr;
    if (isDXT_FileBytes(bytes, length)) {
        auto dxt = loadDXT_FileBytes(bytes, length);
        width = dxt.width;
        height = dxt.height;
        channels = dxt.alpha ? 4 : 3;
        format = dxt.alpha ? ImageFormat::RGBA8_BC3 : ImageFormat::RGB8_BC1;
        data = malloc(dxt.data.size());
        std::memcpy(data, dxt.data.data(), dxt.data.size());
    } else if (isASTC_FileBytes(bytes, length)) {
        auto astc = loadASTC_FileBytes(bytes, length);
        if (astc.blockX == 4 && astc.blockY == 4) format = ImageFormat::RGBA8_ASTC4x4;
        if (astc.blockX == 5 && astc.blockY == 5) format = ImageFormat::RGBA8_ASTC5x5;
        if (astc.blockX == 6 && astc.blockY == 6) format = ImageFormat::RGBA8_ASTC6x6;
        if (astc.blockX == 8 && astc.blockY == 8) format = ImageFormat::RGBA8_ASTC8x8;
        width = astc.width;
        height = astc.height;
        channels = 4;
        data = malloc(astc.data.size());
        std::memcpy(data, astc.data.data(), astc.data.size());
    } else if (stbi_is_hdr_from_memory(bytes, length)) {
        data = stbi_loadf_from_memory(bytes, length, &width, &height, &channels, 0);
        if (!data) {
            throw std::runtime_error("Failed to load image 'F32' from memory");
        }
        format = getImageFormat(channels, 32, true);
    } else {
        // Idk gives weird texture artifacts + How the hell am I supposed to know whether image is 16/8 bits ???
        //data = stbi_load_16_from_memory(bytes, length, &width, &height, &channels, 0);
        if (!data) {
            data = stbi_load_from_memory(bytes, length, &width, &height, &channels, 0);
            if (!data) {
                throw std::runtime_error("Failed to load image 'U8/16' from memory");
            }
            format = getImageFormat(channels, 8, false);
        } else {
            format = getImageFormat(channels, 16, false);
        }
    }
    return data;
}

TextureImage syng::readTextureImage(const std::filesystem::path& path) {
    std::ifstream textureFile;

    textureFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    textureFile.open(path, std::ios::binary);
    std::vector<char> file_bytes((std::istreambuf_iterator<char>(textureFile)), {});
    textureFile.close();

    ImageFormat format;
    int width, height, channels;
    void *data = loadRawImage_File(path, format, width, height, channels);

    std::vector<uint8_t> texel_bytes;

    if (format == ImageFormat::RGB8_BC1 || format == ImageFormat::RGBA8_BC3) {
        int blockBytes = (format == ImageFormat::RGB8_BC1) ? 8 : 16; // BC1 = 8, BC3 = 16
        int blocksX = (width  + 3) / 4;
        int blocksY = (height + 3) / 4;
        size_t dataSize = static_cast<size_t>(blocksX) * blocksY * blockBytes;

        texel_bytes.resize(dataSize);
        std::memcpy(texel_bytes.data(), data, dataSize);
    } else if (static_cast<int>(format) >= ImageFormat::RGBA8_ASTC4x4) {
        int blockDimX, blockDimY;

        switch (format) {
            case ImageFormat::RGBA8_ASTC4x4: blockDimX = 4; blockDimY = 4; break;
            case ImageFormat::RGBA8_ASTC5x5: blockDimX = 5; blockDimY = 5; break;
            case ImageFormat::RGBA8_ASTC6x6: blockDimX = 6; blockDimY = 6; break;
            case ImageFormat::RGBA8_ASTC8x8: blockDimX = 8; blockDimY = 8; break;
            default: throw std::runtime_error("Unsupported ASTC format");
        }

        int blocksX = (width  + blockDimX - 1) / blockDimX;
        int blocksY = (height + blockDimY - 1) / blockDimY;
        size_t dataSize = static_cast<size_t>(blocksX) * blocksY * 16;

        texel_bytes.resize(dataSize);
        std::memcpy(texel_bytes.data(), data, dataSize);
    } else {
        const ImageFormatInfo& info = getImageFormatInfo(format);
        size_t dataSize = static_cast<size_t>(width) * height * info.nrComponents * info.bits();

        texel_bytes.resize(dataSize);
        std::memcpy(texel_bytes.data(), data, dataSize);
    }
    freeImageData(data, format);

    return {format, texel_bytes, width, height};
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
    DataTemplates::write_varint(buffer, type);
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
    if (paths.size() != 6) throw std::runtime_error("readTextureCubemap(): paths.size != 6");
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
    MaterialTexture2D_T texType = static_cast<MaterialTexture2D_T>(DataTemplates::read_varint(buffer));
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

GLuint syng::uploadTex2DFromBytes(void *raw, int length, int width, int height, ImageFormatInfo info, TexelExecParams exec_params) {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLuint TCB;
    GLenum format = info.format, internalFormat = info.internalFormat;
#ifdef __EMSCRIPTEN__
    internalFormat = format;
#endif
    glGenTextures(1, &TCB);
    glBindTexture(GL_TEXTURE_2D, TCB);
    if (info.format >= GL_COMPRESSED_RGBA_ASTC_4x4 || info.format >= GL_COMPRESSED_RGB_S3TC_DXT1_EXT) {
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, (GLsizei) length, raw);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, info.type, raw);
    }
    exec_params(TCB);
    return TCB;
}

std::future<GLuint> syng::uploadTex2DFromBytes_TQ(std::vector<uint8_t> pixels, int width, int height, ImageFormatInfo info, TexelExecParams exec_params) {
    return TaskQueue::Instance().enqueueFuture([pixels = std::move(pixels), width, height, info, exec_params](){
        GLuint TCB = uploadTex2DFromBytes((void*)pixels.data(), pixels.size(), width, height, info, exec_params);
        exec_params(TCB);
        return TCB;
    });
}

GLuint syng::uploadTex2DFromFileBytes(uint8_t *bytes, int length, TexelExecParams exec_params) {
    ImageFormat format;
    int width, height, channels;
    void *data = loadRawImage_FileBytes(static_cast<uint8_t*>(bytes), length, format, width, height, channels);
    uint8_t *data_uc = static_cast<uint8_t*>(data);
    ImageFormatInfo info = getImageFormatInfo(format);
    std::vector<uint8_t> pixelCopy(data_uc, data_uc + width * height * channels * info.bits());
    GLuint TCB = uploadTex2DFromBytes_TQ(pixelCopy, width, height, info, exec_params).get();
    freeImageData(data, format);
    return TCB;
}

GLuint syng::uploadTex2DFromFile(const std::filesystem::path& path, TexelExecParams exec_params) {
    ImageFormat format;
    int width, height, channels;
    void *data = loadRawImage_File(path, format, width, height, channels);
    uint8_t *data_uc = static_cast<uint8_t*>(data);
    ImageFormatInfo info = getImageFormatInfo(format);
    std::vector<uint8_t> pixelCopy(data_uc, data_uc + width * height * channels * info.bits());
    GLuint TCB = uploadTex2DFromBytes_TQ(pixelCopy, width, height, info, exec_params).get();
    freeImageData(data, format);
    return TCB;
}

GLuint syng::uploadTexCubeFromBytes(void **raws, int *lengths, int *widths, int *heights, ImageFormatInfo *infos, TexelExecParams exec_params) {
    GLuint QBTCB;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &QBTCB);
    glBindTexture(GL_TEXTURE_CUBE_MAP, QBTCB);

    for (unsigned int i = 0; i < 6; i++) {
        void *raw = raws[i];
        int width = widths[i], height = heights[i];
        ImageFormatInfo info = infos[i];

        if (!raw) continue;
        if (info.format >= GL_COMPRESSED_RGBA_ASTC_4x4 || info.format >= GL_COMPRESSED_RGB_S3TC_DXT1_EXT) {
            glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, info.format, width, height, 0, (GLsizei) lengths[i], raw);
        } else {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, info.internalFormat,
                width, height, 0, info.format, info.type, raw
            );
        }
    }
    exec_params(QBTCB);
    return QBTCB;
}

std::future<GLuint> syng::uploadTexCubeFromBytes_TQ(std::vector<std::vector<uint8_t>> pixels, int *widths, int *heights, ImageFormatInfo *infos, TexelExecParams exec_params) {
    return TaskQueue::Instance().enqueueFuture([pixels = std::move(pixels), widths, heights, infos, exec_params](){
        const uint8_t** raws;
        int lengths[6];
        for (int i = 0; i < 6; i++) {
            raws[i] = pixels[i].data();
            lengths[i] = pixels[i].size();
        }
        GLuint TCB = uploadTexCubeFromBytes((void**) raws, lengths, widths, heights, infos, exec_params);
        exec_params(TCB);
        return TCB;
    });
}

GLuint syng::uploadTexCubeFromFilesBytes(uint8_t **bytes, int *lengths, TexelExecParams exec_params) {
    uint8_t *raws[6];
    int widths[6], heights[6], nrComps[6];
    ImageFormat fmts[6];
    ImageFormatInfo infos[6];

    for (unsigned int i = 0; i < 6; i++) {
        raws[i] = (uint8_t*) loadRawImage_FileBytes(bytes[i], lengths[i], fmts[i], widths[i], heights[i], nrComps[i]);
        infos[i] = getImageFormatInfo(fmts[i]);
    }
    std::vector<std::vector<uint8_t>> pixelCopyQB;
    for (int i = 0; i < 6; i++) {
        uint8_t* data_uc = raws[i];
        std::vector<uint8_t> pixelCopy(data_uc, data_uc + widths[i] * heights[i] * nrComps[i] * infos[i].bits());
        pixelCopyQB.push_back(pixelCopy);
        freeImageData(data_uc, fmts[i]);
    }
    GLuint QBTCB = uploadTexCubeFromBytes_TQ(pixelCopyQB, widths, heights, infos, exec_params).get();
    return QBTCB;
}

GLuint syng::uploadTexCubeFromFiles(std::vector<std::filesystem::path>& paths, TexelExecParams exec_params) {
    uint8_t *raws[6];
    int widths[6], heights[6], nrComps[6];
    ImageFormat fmts[6];
    ImageFormatInfo infos[6];

    for (unsigned int i = 0; i < 6; i++) {
        raws[i] = (uint8_t*) loadRawImage_File(paths[i], fmts[i], widths[i], heights[i], nrComps[i]);
        infos[i] = getImageFormatInfo(fmts[i]);
    }
    std::vector<std::vector<uint8_t>> pixelCopyQB;
    for (int i = 0; i < 6; i++) {
        uint8_t* data_uc = raws[i];
        std::vector<uint8_t> pixelCopy(data_uc, data_uc + widths[i] * heights[i] * nrComps[i] * infos[i].bits());
        pixelCopyQB.push_back(pixelCopy);
        freeImageData(data_uc, fmts[i]);
    }
    GLuint QBTCB = uploadTexCubeFromBytes_TQ(pixelCopyQB, widths, heights, infos, exec_params).get();
    return QBTCB;
}