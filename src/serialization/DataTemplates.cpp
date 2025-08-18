#include "DataTemplates.hpp"
#include "DataSerializer.hpp"
#include "modules/Mesh.hpp"
#include <stdexcept>

namespace syng
{
namespace DataTemplates
{
void push(DataDeserializer *buffer, std::string res, uint16_t header) {
    uint16_t read = read_uint16(buffer);
    if (read != header) {
        buffer->rewind(2);
        throw std::runtime_error("Invalid Header (" + res + "): read=" + std::to_string(read));
    }
}

void pop(DataDeserializer *buffer, std::string res, uint16_t footer) {
    uint16_t read = read_uint16(buffer);
    if (read != footer) {
        throw std::runtime_error("Invalid Footer (" + res + "): " + std::to_string(read));
    }
}

void write_mesh_material(DataSerializer *buffer, MaterialProps material) {
    write_glm_vec3(buffer, material.ior);
    write_float(buffer, material.shininess);
    write_float(buffer, material.minOpacity);
    write_float(buffer, material.maxOpacity);
    write_float(buffer, material.opacity);
    write_float(buffer, material.F0);
    uint8_t bool_bytes[3] = {material.isTransparent, material.hasDisplacement, material.hasRoughness};
    buffer->write(bool_bytes, 3);
}

void write_string(DataSerializer* buff, const std::string& value) {
    LittleEndian::write<uint16_t>(buff, value.length());
    buff->write(reinterpret_cast<const uint8_t*>(value.c_str()), value.length());
}

void write_glm_mat3(DataSerializer *buffer, const glm::mat3& val) {
    for (uint32_t i = 0; i < 3; i++) {
        for (uint32_t j = 0; j < 3; j++) {
            LittleEndian::write(buffer, val[i][j]);
        }
    }
}

void write_glm_mat4(DataSerializer *buffer, const glm::mat4& val) {
    for (uint32_t i = 0; i < 4; i++) {
        for (uint32_t j = 0; j < 4; j++) {
            LittleEndian::write(buffer, val[i][j]);
        }
    }
}

void write_glm_vec3(DataSerializer *buffer, const glm::vec3& val) {
    for (uint32_t i = 0; i < 3; i++) {
        LittleEndian::write(buffer, val[i]);
    }
}

void write_glm_vec2(DataSerializer *buffer, const glm::vec2& val) {
    for (uint32_t i = 0; i < 2; i++) {
        LittleEndian::write(buffer, val[i]);
    }
}

MaterialProps read_mesh_material(DataDeserializer *buffer) {
    MaterialProps res;
    res.ior = read_glm_vec3(buffer);
    res.shininess = read_float(buffer);
    res.minOpacity = read_float(buffer);
    res.maxOpacity = read_float(buffer);
    res.opacity = read_float(buffer);
    res.F0 = read_float(buffer);
    uint8_t bool_bytes[3];
    buffer->read(bool_bytes, 3);
    res.isTransparent = bool_bytes[0];
    res.hasDisplacement = bool_bytes[1];
    res.hasRoughness = bool_bytes[2];
    return res;
}

std::string read_string(DataDeserializer* buff) {
    uint16_t length = LittleEndian::read<uint16_t>(buff);
    uint8_t cstr[length];
    buff->read(cstr, length);
    return std::string(reinterpret_cast<char*>(cstr), length);
}

glm::mat3 read_glm_mat3(DataDeserializer *buffer) {
    glm::mat3 result(1.0f);
    for (uint32_t i = 0; i < 3; i++) {
        for (uint32_t j = 0; j < 3; j++) {
            result[i][j] = LittleEndian::read<float>(buffer);
        }
    }
    return result;
}

glm::mat4 read_glm_mat4(DataDeserializer *buffer) {
    glm::mat4 result(1.0f);
    for (uint32_t i = 0; i < 4; i++) {
        for (uint32_t j = 0; j < 4; j++) {
            result[i][j] = LittleEndian::read<float>(buffer);
        }
    }
    return result;
}

glm::vec3 read_glm_vec3(DataDeserializer *buffer) {
    glm::vec3 result(1.0f);
    for (uint32_t i = 0; i < 3; i++) {
        result[i] = LittleEndian::read<float>(buffer);
    }
    return result;
}

glm::vec2 read_glm_vec2(DataDeserializer *buffer) {
    glm::vec2 result(1.0f);
    for (uint32_t i = 0; i < 2; i++) {
        result[i] = LittleEndian::read<float>(buffer);
    }
    return result;
}
}
}