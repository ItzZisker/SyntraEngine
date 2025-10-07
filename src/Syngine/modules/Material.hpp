#pragma once

#include "Texture.hpp"

#include <glm/glm.hpp>

#include <unordered_map>
#include <variant>
#include <vector>
#include <string>

using namespace syng;

struct MaterialProps {
    glm::vec3 ior = glm::vec3(1.0f);
    float shininess = 32.0f;
    float minOpacity = 0.7f, maxOpacity = 1.0f; // Used if dynamic opacity is enabled within shader
    float opacity = 1.0f;
    float F0 = 0.04f;
    bool isTransparent = false;
    bool hasDisplacement = true;
    bool hasRoughness = true;
};

class M_Metadata {
public:
    const int type;

    M_Metadata(int type, const char *data, int data_length);

    using Value = std::variant<float, double, std::string, int, std::vector<char>>;
    Value getValue() const;

    std::vector<char>& rawData();
    int dataLength();
private:
    std::vector<char> data_copy;
    const int data_length;
};

using MetaDataMap = std::unordered_map<std::string, M_Metadata>;

namespace FallbackTexture
{
    extern Texture2D Diffuse, Specular, Normal, Height, Rough;
    Texture2D get(MaterialTexture2D_T type);
}

using TexelByTypeMap = std::unordered_map<MaterialTexture2D_T, std::vector<Texture2D>>;

class Material {
private:
    int ID = -1;
    std::string name = "Default";
public:
    MaterialProps props = {};
    MetaDataMap metadata_map;
    TexelByTypeMap textures;

    Material();
    Material(int id, std::string name, MaterialProps props, MetaDataMap metadata);

    bool hasTexture(MaterialTexture2D_T type);
    std::vector<Texture2D>& getTextures(MaterialTexture2D_T type, std::vector<Texture2D>& _default);
    std::vector<Texture2D>& getTextures(MaterialTexture2D_T type);
    void addTexture(MaterialTexture2D_T type, Texture2D texel);
    void delTexture(MaterialTexture2D_T type, std::string& path);

    int getID();
    std::string& getName();
};

namespace FallbackMaterial
{
    extern Material* Default;
}