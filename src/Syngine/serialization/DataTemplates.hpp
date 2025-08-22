#pragma once

#include "DataSerializer.hpp"
#include "glm/glm.hpp"
#include "Syngine/modules/Mesh.hpp"
#include <functional>
#include <vector>

namespace syng
{
namespace DataTemplates
{
void push(DataDeserializer *buffer, std::string res, uint16_t header);
void pop(DataDeserializer *buffer, std::string res, uint16_t footer);

constexpr void write_bool(DataSerializer* buffer, bool b) { uint8_t bytes[1] = {b}; buffer->write(bytes, 1); }
constexpr void write_int32(DataSerializer* buffer, int32_t i) { LittleEndian::write<int32_t>(buffer, i); };
constexpr void write_int64(DataSerializer* buffer, int64_t i) { LittleEndian::write<int64_t>(buffer, i); };
constexpr void write_uint16(DataSerializer* buffer, uint16_t i) { LittleEndian::write<uint16_t>(buffer, i); };
constexpr void write_uint32(DataSerializer* buffer, uint32_t i) { LittleEndian::write<uint32_t>(buffer, i); };
constexpr void write_uint64(DataSerializer* buffer, uint64_t i) { LittleEndian::write<uint64_t>(buffer, i); };
constexpr void write_float(DataSerializer* buffer, float f) { LittleEndian::write<float>(buffer, f); };

void write_mesh_material(DataSerializer* buffer, MaterialProps material);
void write_string(DataSerializer* buffer, const std::string& value);
void write_glm_mat3(DataSerializer* buffer, const glm::mat3& val);
void write_glm_mat4(DataSerializer* buffer, const glm::mat4& val);
void write_glm_vec3(DataSerializer* buffer, const glm::vec3& val);
void write_glm_vec2(DataSerializer* buffer, const glm::vec2& val);

constexpr bool read_bool(DataDeserializer* buffer) { return buffer->readByte(); }
constexpr int32_t read_int32(DataDeserializer* buffer) { return LittleEndian::read<int32_t>(buffer); };
constexpr int64_t read_int64(DataDeserializer* buffer) { return LittleEndian::read<int64_t>(buffer); };
constexpr uint16_t read_uint16(DataDeserializer* buffer) { return LittleEndian::read<uint16_t>(buffer); };
constexpr uint32_t read_uint32(DataDeserializer* buffer) { return LittleEndian::read<uint32_t>(buffer); };
constexpr uint64_t read_uint64(DataDeserializer* buffer) { return LittleEndian::read<uint64_t>(buffer); };
constexpr float read_float(DataDeserializer* buffer) { return LittleEndian::read<float>(buffer); };

MaterialProps read_mesh_material(DataDeserializer* buffer);
std::string read_string(DataDeserializer* buffer);
glm::mat3 read_glm_mat3(DataDeserializer* buffer);
glm::mat4 read_glm_mat4(DataDeserializer* buffer);
glm::vec3 read_glm_vec3(DataDeserializer* buffer);
glm::vec2 read_glm_vec2(DataDeserializer* buffer);

template<typename T>
void write_vector(DataSerializer* buffer, const std::vector<T>& vector, std::function<void(const T& element)> write_element) {
    LittleEndian::write<uint64_t>(buffer, vector.size());
    for (auto& element : vector) { write_element(element); }
}

template<typename T>
std::vector<T> read_vector(DataDeserializer* buffer, std::function<T()> read_element) {
    std::vector<T> res;
    uint64_t size = LittleEndian::read<uint64_t>(buffer);
    for (uint64_t i = 0; i < size; i++) { res.push_back(read_element()); }
    return res;
}

}
}