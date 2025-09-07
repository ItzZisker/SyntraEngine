#pragma once

#include "Syngine/utils/GameUtils.hpp"

#include <stdexcept>
#include <vector>

namespace syng
{
enum GLAttributePointer {
    GLPointer_Default,
    GLPointer_Int32,
    GLPointer_Int64
};

struct GLVertexAttribute {
    GLuint index = 0;
    GLint size = 3;
    GLenum type = GL_FLOAT;
    GLboolean normalized = GL_FALSE;
    GLsizei stride = 3 * sizeof(float);
    void* pointer = (void*) 0;
    GLAttributePointer pointerType = GLPointer_Default;
};

struct GLDataSub {
    GLintptr offset;
    GLsizeiptr size;
    void* data = (void*) 0;
};

template<typename V = GLfloat>
class GLVertex {
protected:
    std::vector<V> vertices;
    std::vector<GLVertexAttribute> attributes;
    std::vector<GLDataSub> dataSubs;

    GLuint VAO = 0, VBO = 0;
    int componentsPerVertex;
public:
    GLVertex(std::vector<V> vertices, int componentsPerVertex = 3)
        : vertices(vertices), componentsPerVertex(componentsPerVertex) {}

    ~GLVertex() {
        if (VBO) glDeleteBuffers(1, &VBO);
        if (VAO) glDeleteVertexArrays(1, &VAO);
    }

    void attribute(GLVertexAttribute attribute) {
        if (!isReserved()) attributes.push_back(attribute);
    }

    void dataSub(GLDataSub dataSub) {
        if (!isReserved()) dataSubs.push_back(dataSub);
    }

    virtual bool isReserved() {
        return VAO != 0;
    }

    virtual bool isLoaded() {
        return isReserved();
    }

    virtual void reserve() {
        if (isReserved()) return;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        GLsizeiptr bufferSize = 0;
        if (!dataSubs.empty()) {
            for (const GLDataSub &sub : dataSubs) {
                GLintptr end = sub.offset + static_cast<GLintptr>(sub.size);
                if (end > bufferSize) bufferSize = end;
            }
        } else {
            bufferSize = static_cast<GLsizeiptr>(vertices.size() * sizeof(V));
        }

        glBufferData(GL_ARRAY_BUFFER, bufferSize, dataSubs.empty() ? &vertices[0] : nullptr, GL_STATIC_DRAW);

        for (const GLDataSub &sub : dataSubs) {
            glBufferSubData(GL_ARRAY_BUFFER, sub.offset, sub.size, sub.data);
        }
        if (attributes.empty()) {
            attributes.push_back({0, componentsPerVertex});
        }
        for (GLVertexAttribute att : attributes) {
            switch (att.pointerType) {
                case GLPointer_Default:
                    glVertexAttribPointer(att.index, att.size, att.type, att.normalized, att.stride, att.pointer);
                break;
                case GLPointer_Int32:
                    glVertexAttribIPointer(att.index, att.size, att.type, att.stride, att.pointer);
                break;
                case GLPointer_Int64:
                    throw std::runtime_error("GLPointer_Int64 is unsupported on GL330");
                break;
                default:
                    throw std::runtime_error("Unknown pointerType in attribute");
            }
            glEnableVertexAttribArray(att.index);
        }
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    virtual void draw(GLenum mode = GL_TRIANGLES, GLuint count = 0) {
        if (!isReserved()) return;
        glBindVertexArray(VAO);
        glDrawArrays(mode, 0, count != 0 ? count : vertices.size() / componentsPerVertex);
        glBindVertexArray(0);
    }

    GLuint getVAO() { return VAO; }
    GLuint getVBO() { return VBO; }

    std::vector<V>& getVertices() { return this->vertices; }
};

template<typename V = GLfloat>
class GLVertexElement : public GLVertex<V> {
protected:
    std::vector<GLuint> indices;
    GLuint EBO = 0;
public:
    GLVertexElement(std::vector<V> vertices, std::vector<GLuint> indices, int componentsPerVertex = 3)
        : GLVertex<V>(vertices, componentsPerVertex), indices(indices) {}

    ~GLVertexElement() {
        if (EBO) glDeleteBuffers(1, &EBO);
    }

    bool isReserved() override {
        return this->VAO != 0 && EBO != 0;
    }
    
    bool isLoaded() override {
        return GLVertexElement::isReserved();
    }
    
    void reserve() override {
        if (isReserved()) return;
        GLVertex<V>::reserve();
        glBindVertexArray(this->VAO);
        glGenBuffers(1, &this->EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void draw(GLenum mode = GL_TRIANGLES, GLuint count = 0) override {
        if (!isReserved()) return;
        glBindVertexArray(this->VAO);
        glDrawElements(mode, count != 0 ? count : static_cast<GLuint>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    GLuint getEBO() { return this->EBO; }

    std::vector<GLuint>& getIndices() { return this->indices; }
};
}