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
    Texture2D Metal = {};
    Texture2D Emissive = {};
    Texture2D AO = {};
}

Texture2D FallbackTexture::get(MaterialTexture2D_T type) {
    switch (type) {
        case Texture_Diffuse: return FallbackTexture::Diffuse;
        case Texture_Specular: return FallbackTexture::Specular;
        case Texture_Normal: return FallbackTexture::Normal;
        case Texture_Height: return FallbackTexture::Height;
        case Texture_Rough: return FallbackTexture::Rough;
        case Texture_Metallic: return FallbackTexture::Metal;
        case Texture_Emissive: return FallbackTexture::Emissive;
        case Texture_AmbientOcclusion: return FallbackTexture::AO;
        default: return {};
    }
}

namespace FallbackMaterial
{
    Material *Default = new Material(false);
    Material *Default_PBR = new Material(true);
};

Material::Material(bool PBR) : pbr(PBR) {}
Material::Material(int id, std::string name, MaterialProps props, MetaDataMap metadata, bool pbr)
    : ID(id), name(name), props(props), metadata_map(metadata), pbr(pbr) {}

Material::~Material() {
    for (auto& [type, texels] : textures) {
        for (Texture2D& texel : texels) {
            delete &texel;
        }
    }
    metadata_map.clear();
    textures.clear();
}

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

void Material::remTexture(MaterialTexture2D_T type, std::string& path, std::function<bool(Texture2D& tex)> remove_if) {
    if (!hasTexture(type)) return;
    std::vector<Texture2D>& texels = textures[type];
    auto it = std::remove_if(
        texels.begin(),
        texels.end(),
        remove_if
    );
    texels.erase(it, texels.end());
}

void Material::popTexture(MaterialTexture2D_T type, std::string& path) {
    remTexture(type, path, [path](Texture2D& tex) {
        return tex.getPath() == path;
    });
}

void Material::delTexture(MaterialTexture2D_T type, std::string& path) {
    remTexture(type, path, [path](Texture2D& tex) {
        bool shouldRemove = tex.getPath() == path;
        if (shouldRemove) tex.deleteTexture();
        return shouldRemove;
    });
}

bool Material::isPBR() {
    return this->pbr;
}

int Material::getID() {
    return this->ID;
}

std::string& Material::getName() {
    return this->name;
}