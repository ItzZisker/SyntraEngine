#pragma once

#include <cstdint>
#include <cstring>
#include <type_traits>
#include <string>
#include <vector>

namespace syng
{
class DataSerializer {
private:
    uint8_t* data;
    uint64_t writeIndex;
    const uint64_t length;
public:
    DataSerializer(uint64_t length);
    ~DataSerializer();

    std::vector<uint8_t> copyData(uint64_t end) const { return std::vector<uint8_t>(data, data + end); }
    std::vector<uint8_t> copyData() const { return copyData(length); }
    uint64_t getLength() const { return length; }
    uint64_t getWritePos() const { return writeIndex; }

    void write(const unsigned char* bytes, size_t size);
    void rewind(uint64_t pos);
};

class DataDeserializer {
private:
    const uint8_t* data;
    const uint64_t length;
    uint64_t readIndex;
public:
    DataDeserializer(const uint8_t* buffer, uint64_t length);

    uint64_t getLength() const { return length; }
    uint64_t getReadPos() const { return readIndex; }

    uint8_t readByte();
    void read(unsigned char* outBytes, size_t size);
    void rewind(uint64_t pos);
    void skip(uint64_t size);
};

namespace LittleEndian
{
template<typename T>
void write(DataSerializer* buff, T value) {
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);
    uint8_t bytes[sizeof(T)];
    std::memcpy(bytes, &value, sizeof(T));
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    std::reverse(bytes, bytes + sizeof(T));
#endif
    buff->write(reinterpret_cast<unsigned char*>(bytes), sizeof(T));
}

template<typename T>
T read(DataDeserializer* buff) {
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);
    uint8_t bytes[sizeof(T)];
    buff->read(reinterpret_cast<unsigned char*>(bytes), sizeof(T));
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    std::reverse(bytes, bytes + sizeof(T));
#endif
    T value{};
    std::memcpy(&value, bytes, sizeof(T));
    return value;
}
}

}