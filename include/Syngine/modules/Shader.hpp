#pragma once

#include <glad/glad.h>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
    unsigned int ID;
    const char *vertexPath, *fragmentPath;

    Shader(const char *vertexPath, const char *fragmentPath);

    void init();

    void use();

    void disposeProgram();

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
};