#include "Shader.hpp"

#include "Syngine/serialization/DataTemplates.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

using namespace syng;

ShaderEncoder::ShaderEncoder(DataSerializer *buffer) : buffer(buffer) {}

void ShaderEncoder::encode(std::string vertex, std::string fragment, std::string geometry) {
    DataTemplates::write_uint16(buffer, PCK_HEADER_SHADER);
    DataTemplates::write_string(buffer, vertex);
    DataTemplates::write_string(buffer, fragment);
    DataTemplates::write_bool(buffer, !geometry.empty());
    if (!geometry.empty()) {
        DataTemplates::write_string(buffer, geometry);
    }
    DataTemplates::write_uint16(buffer, PCK_FOOTER_SHADER);
}

void ShaderEncoder::encode(Shader shader) {
    ShaderEncoder::encode(shader.vertexCode, shader.fragmentCode, shader.geometryCode);
}

ShaderDecoder::ShaderDecoder(DataDeserializer *buff) : buffer(buff) {}

Shader LazyShader::publish() {
    return {*this};
}

LazyShader ShaderDecoder::decodeLazy() {
    DataTemplates::push(buffer, "Shader", PCK_HEADER_SHADER);

    LazyShader lazy;
    lazy.vertexCode = DataTemplates::read_string(buffer);
    lazy.fragmentCode = DataTemplates::read_string(buffer);

    if (DataTemplates::read_bool(buffer)) {
        lazy.geometryCode = DataTemplates::read_string(buffer);
    }
    DataTemplates::pop(buffer, "Shader", PCK_FOOTER_SHADER);
    return lazy;
}

Shader ShaderDecoder::decode() {
    return ShaderDecoder::decodeLazy().publish();
}

Shader::Shader(LazyShader lazy) : vertexCode(lazy.vertexCode), fragmentCode(lazy.fragmentCode), geometryCode(lazy.geometryCode) {}

void Shader::read(DataDeserializer *buffer) {
    LazyShader lazy = ShaderDecoder(buffer).decodeLazy();
    this->vertexCode = lazy.vertexCode;
    this->fragmentCode = lazy.fragmentCode;
    this->geometryCode = lazy.geometryCode;
}

void Shader::read(std::filesystem::path vertexPath, std::filesystem::path fragmentPath, std::filesystem::path geometryPath) {
    std::ifstream vShaderFile, fShaderFile, gShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    if (!geometryPath.empty()) {
        gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    }

    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);

        if (!geometryPath.empty()) {
            gShaderFile.open(geometryPath);
        }

        std::stringstream vShaderStream, fShaderStream, gShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        if (gShaderFile.is_open()) {
            gShaderStream << gShaderFile.rdbuf();
        }

        vShaderFile.close();
        fShaderFile.close();

        if (gShaderFile.is_open()) {
            gShaderFile.close();
        }

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

        if (!geometryPath.empty()) {
            geometryCode = gShaderStream.str();
        } else {
            geometryCode = "";
        }
    } catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ::" << e.what() << ", ERR=" << e.code() << std::endl;
    }
}

void Shader::init(std::map<std::string, std::string> variables) {
    this->variables = variables;
    
    for (const auto& pair : variables) {
        std::regex pattern("\\$\\{" + pair.first + "=(.+?)\\}");

        vertexCode = std::regex_replace(vertexCode, pattern, pair.second);
        fragmentCode = std::regex_replace(fragmentCode, pattern, pair.second);

        if (!geometryCode.empty()) {
            geometryCode = std::regex_replace(geometryCode, pattern, pair.second);
        }
    }

    std::regex pattern(R"(\$\{[^=]+=(.+?)\})");

    vertexCode = std::regex_replace(vertexCode, pattern, "$1");
    fragmentCode = std::regex_replace(fragmentCode, pattern, "$1");

    if (!geometryCode.empty()) {
        geometryCode = std::regex_replace(fragmentCode, pattern, "$1");
    }

    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();
    const char *gShaderCode = !geometryCode.empty() ? geometryCode.c_str() : nullptr;

    GLuint vertex, fragment, geometry = 0;
    int success;
    char infoLog[512];

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    };

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    if (gShaderCode != nullptr) {
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);

        glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(geometry, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }

    ID = glCreateProgram();

    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if (geometry) glAttachShader(ID, geometry);
    glLinkProgram(ID);

    glGetProgramiv(ID, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometry) glDeleteShader(geometry);
}

std::string Shader::getVariable(std::string key) {
    const auto& pair = variables.find(key);
    return pair == variables.end() ? "" : pair->second;
}

void Shader::reloadProgram(std::map<std::string, std::string> variables) {
    disposeProgram();
    init(variables);
}

void Shader::disposeProgram() {
    glDeleteProgram(ID);
    ID = 0;
}

void Shader::use() {
    glUseProgram(ID);
}

void Shader::setTexture(const std::string &name, int type, int index, int TCB) const {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(type, TCB);
    glUniform1i(glGetUniformLocation(ID, name.c_str()), index);
}

void Shader::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec2f(const std::string &name, float x, float y) {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}

void Shader::setVec3f(const std::string &name, float x, float y, float z) {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::setVec4f(const std::string &name, float x, float y, float z, float w) {
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Shader::setVec2f(const std::string &name, glm::vec2 vec) {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), vec[0], vec[1]);
}

void Shader::setVec3f(const std::string &name, glm::vec3 vec) {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), vec[0], vec[1], vec[2]);
}

void Shader::setVec4f(const std::string &name, glm::vec4 vec) {
    glUniform4f(glGetUniformLocation(ID, name.c_str()), vec[0], vec[1], vec[2], vec[3]);
}

void Shader::setMatrix3(const std::string &name, glm::mat3 matrix, int count, bool transpose) {
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), count, transpose, glm::value_ptr(matrix));
}

void Shader::setMatrix4(const std::string &name, glm::mat4 matrix, int count, bool transpose) {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), count, transpose, glm::value_ptr(matrix));
}