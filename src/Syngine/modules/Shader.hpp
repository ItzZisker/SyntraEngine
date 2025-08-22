#pragma once

#include "Syngine/serialization/DataSerializer.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <string>
#include <map>

namespace syng
{
constexpr uint16_t PCK_HEADER_SHADER = 102;
constexpr uint16_t PCK_FOOTER_SHADER = 103;

class Shader;

class LazyShader {
public:
    std::string vertexCode, fragmentCode, geometryCode = "";
    Shader publish();
};

class ShaderEncoder {
private:
    DataSerializer *buffer;
public:
    ShaderEncoder(DataSerializer *buffer);
    void encode(std::string vertex, std::string fragment, std::string geometry = "");
    void encode(Shader shader);
};

class ShaderDecoder {
private:
    DataDeserializer *buffer;
public:
    ShaderDecoder(DataDeserializer *buffer);
    LazyShader decodeLazy();
    Shader decode();
};

class Shader
{
private:
    std::map<std::string, std::string> variables;
    GLuint ID = 0;
public:
    std::string vertexCode, fragmentCode, geometryCode;

    Shader() = default;
    Shader(LazyShader lazy);
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept
        : variables(std::move(other.variables)),
          ID(other.ID),
          vertexCode(std::move(other.vertexCode)),
          fragmentCode(std::move(other.fragmentCode)),
          geometryCode(std::move(other.geometryCode)) {
        other.ID = 0;
    }

    Shader& operator=(Shader&& other) noexcept {
        if (this != &other) {
            disposeProgram();
            variables = std::move(other.variables);
            ID = other.ID;
            vertexCode = std::move(other.vertexCode);
            fragmentCode = std::move(other.fragmentCode);
            geometryCode = std::move(other.geometryCode);
            other.ID = 0;
        }
        return *this;
    }

    std::string getVariable(std::string key);

    void read(DataDeserializer *buffer);
    void read(std::filesystem::path vertexPath, std::filesystem::path fragmentPath, std::filesystem::path geometryPath = "");
    void init(std::map<std::string, std::string> variables = {});
    void use();
    
    void reloadProgram(std::map<std::string, std::string> variables = {});
    void disposeProgram();

    void setTexture(const std::string &name, int textureType, int index, int TCB) const;
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec2f(const std::string &name, float x, float y);
    void setVec3f(const std::string &name, float x, float y, float z);
    void setVec4f(const std::string &name, float x, float y, float z, float w);
    void setVec2f(const std::string &name, glm::vec2 vec);
    void setVec3f(const std::string &name, glm::vec3 vec);
    void setVec4f(const std::string &name, glm::vec4 vec);
    void setMatrix3(const std::string &name, glm::mat3 matrix, int count, bool transpose);
    void setMatrix4(const std::string &name, glm::mat4 matrix, int count, bool transpose);

    unsigned int getProgramID() const { return ID; }
    bool hasProgram() const { return static_cast<bool>(ID); }
};
}