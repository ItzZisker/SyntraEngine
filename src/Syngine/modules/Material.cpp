#include "Material.hpp"
#include "Texture.hpp"

#include <vector>

using namespace syng;

M_Metadata::M_Metadata(int type, const char *data, int data_length)
    : type(type), data_length(data_length) {
    data_copy.resize(data_length);
    if (data && data_length > 0) {
        std::memcpy(data_copy.data(), data, data_length);
    }
}

int M_Metadata::dataLength() {
    return data_length;
}

std::vector<char>& M_Metadata::rawData() {
    return data_copy;
}

M_Metadata::Value M_Metadata::getValue() const {
    const char *raw = data_copy.data();

    switch (type) {
        case 0x1: { // Float
            float val{};
            if (data_length >= sizeof(float)) {
                std::memcpy(&val, raw, sizeof(float));
            }
            return val;
        }
        case 0x2: { // Double
            double val{};
            if (data_length >= sizeof(double)) {
                std::memcpy(&val, raw, sizeof(double));
            }
            return val;
        }
        case 0x3: { // String
            return std::string(raw, data_length);
        }
        case 0x4: { // Integer
            int val{};
            if (data_length >= sizeof(int)) {
                std::memcpy(&val, raw, sizeof(int));
            }
            return val;
        }
        case 0x5: { // Buffer
            return data_copy;
        }
        default:
            throw std::runtime_error("Unknown metadata type");
    }
}

namespace FallbackTexture
{
    Texture2D Diffuse = {};
    Texture2D Specular = {};
    Texture2D Normal = {};
    Texture2D Height = {};
    Texture2D Rough = {};
}

Texture2D FallbackTexture::get(MaterialTexture2D_T type) {
    switch (type) {
        case Texture_Diffuse: return FallbackTexture::Diffuse;
        case Texture_Specular: return FallbackTexture::Specular;
        case Texture_Normal: return FallbackTexture::Normal;
        case Texture_Height: return FallbackTexture::Height;
        case Texture_Rough: return FallbackTexture::Rough;
        default: return {0, "NONE"};
    }
}

namespace FallbackMaterial
{
    Material *Default = new Material();
};

Material::Material() {}
Material::Material(int id, std::string name, MaterialProps props, MetaDataMap metadata) : ID(id), name(name), props(props), metadata_map(metadata) {}

bool Material::hasTexture(MaterialTexture2D_T type) {
    return textures.find(type) != textures.end();
}

std::vector<Texture2D>& Material::getTextures(MaterialTexture2D_T type, std::vector<Texture2D>& _default) {
    return hasTexture(type) ? textures[type] : _default;
}

std::vector<Texture2D>& Material::getTextures(MaterialTexture2D_T type) {
    static std::vector<Texture2D> empty = {};
    return getTextures(type, empty);
}

void Material::addTexture(MaterialTexture2D_T type, Texture2D texture) {
    if (!hasTexture(type)) textures.insert({type, {}});
    textures[type].push_back(texture);
}

void Material::delTexture(MaterialTexture2D_T type, std::string& path) {
    if (!hasTexture(type)) return;
    std::vector<Texture2D>& texels = textures[type];
    auto it = std::remove_if(
        texels.begin(),
        texels.end(),
        [path](const Texture2D& tex) {
            return tex.path == path;
        }
    );
    texels.erase(it, texels.end());
}

int Material::getID() {
    return this->ID;
}

std::string& Material::getName() {
    return this->name;
}