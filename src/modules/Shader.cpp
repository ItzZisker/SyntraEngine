#include <modules/Shader.hpp>

#include <cstdlib>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <regex>
#include <sstream>
#include <iostream>
#include <string>

using namespace syng;

Shader::Shader(const char *vertexPath, const char *fragmentPath) {
    this->vertexPath = vertexPath;
    this->fragmentPath = fragmentPath;
}

void Shader::init(std::map<std::string, std::string> variables) {
    this->variables = variables;

    std::string vertexCode, fragmentCode;
    std::ifstream vShaderFile, fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);

        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

        for (const auto& pair : variables) {
            std::regex pattern("\\$\\{" + pair.first + "=(.+?)\\}");

            vertexCode = std::regex_replace(vertexCode, pattern, pair.second);
            fragmentCode = std::regex_replace(fragmentCode, pattern, pair.second);
        }

        std::regex pattern(R"(\$\{[^=]+=(.+?)\})");

        vertexCode = std::regex_replace(vertexCode, pattern, "$1");
        fragmentCode = std::regex_replace(fragmentCode, pattern, "$1");
    } catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ::" << e.what() << ", Code: " << e.code() << std::endl;
    }

    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    GLuint vertex, fragment;
    int success;
    char infoLog[512];

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << vertexPath << ": ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    };

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cout << fragmentPath << ": ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    ID = glCreateProgram();

    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);

    glGetProgramiv(ID, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
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