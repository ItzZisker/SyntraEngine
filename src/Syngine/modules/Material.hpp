#pragma once

#include "Syngine/modules/Texture.hpp"
#include "Texture.hpp"

#include <functional>
#include <glm/glm.hpp>

#include <unordered_map>
#include <variant>
#include <vector>
#include <string>

using namespace syng;

struct PBRProps {
    glm::vec4 metallicRoughnessNormalOcclusion = glm::vec4(1.0f);
    glm::vec4 emissiveColor = glm::vec4(1.0f);
    float transparencyFactor = 0.0f;
    float alphaTest = 0.0f;
};

struct MaterialProps {
    glm::vec3 ior = glm::vec3(1.0f); // refraction factor
    float shininess = 32.0f; // Used if material lacks roughness AKA inverse-shininess map
    float minOpacity = 0.7f, maxOpacity = 1.0f; // Used if dynamic opacity is enabled within shader
    float opacity = 1.0f; // Static opacity, enabled by default
    float F0 = 0.04f; // Fresnel refraction factor
    bool isTransparent = false;
    bool hasDisplacement = true;
    bool hasRoughness = true;
    glm::vec4 baseColor = glm::vec4(1.0f);
    glm::vec4 diffuseColor = glm::vec4(1.0f);
    glm::vec4 specularColor = glm::vec4(1.0f);
    PBRProps pbr;
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
    extern Texture2D Diffuse, Specular, Normal, Height, Rough, Metal, Emissive, AO;
    Texture2D get(MaterialTexture2D_T type);
}

using TexelByTypeMap = std::unordered_map<MaterialTexture2D_T, std::vector<Texture2D>>;

class Material {
private:
    int ID = -1;
    std::string name = "Default";
    bool pbr = false;

    void remTexture(MaterialTexture2D_T type, std::string& path, std::function<bool(Texture2D& tex)> remove_if);
public:
    MaterialProps props = {};
    MetaDataMap metadata_map;
    TexelByTypeMap textures;

    Material(bool PBR = false);
    Material(int id, std::string name, MaterialProps props, MetaDataMap metadata, bool pbr = false);
    ~Material();

    bool hasTexture(MaterialTexture2D_T type);
    
    std::vector<Texture2D>& getTextures(MaterialTexture2D_T type, std::vector<Texture2D>& _default);
    std::vector<Texture2D>& getTextures(MaterialTexture2D_T type);

    void addTexture(MaterialTexture2D_T type, Texture2D texel);
    void popTexture(MaterialTexture2D_T type, std::string& path);
    void delTexture(MaterialTexture2D_T type, std::string& path);

    bool isPBR();
    int getID();
    std::string& getName();
};

namespace FallbackMaterial
{
    extern Material* Default;
    extern Material* Default_PBR;
}