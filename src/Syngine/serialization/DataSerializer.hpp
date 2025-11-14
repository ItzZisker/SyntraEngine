#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <type_traits>
#include <filesystem>
#include <fstream>

namespace syng
{

class DataStream {
public:
    virtual ~DataStream() = default;

    virtual uint64_t getReadIndex() = 0;
    virtual uint64_t getWriteIndex() = 0;

    virtual void seekRead(uint64_t pos) = 0;
    virtual void skipRead(int64_t offset) = 0;
    virtual void seekWrite(uint64_t pos) = 0;
    virtual void skipWrite(int64_t offset) = 0;
    virtual bool endOfStream() const = 0;

    virtual size_t read(void* out, size_t size) = 0;
    virtual size_t write(const void* in, size_t size) = 0;

    virtual uint64_t getLength() { return 0; }
};

class FileDataStream : public DataStream {
private:
    std::fstream file;
    bool readable = false;
    bool writable = false;
    uint64_t cachedLength = 0;
public:
    FileDataStream(const std::filesystem::path& path, bool read, bool write);
    ~FileDataStream() override;

    uint64_t getReadIndex() override;
    uint64_t getWriteIndex() override;

    void seekRead(uint64_t pos) override;
    void skipRead(int64_t offset) override;
    void seekWrite(uint64_t pos) override;
    void skipWrite(int64_t offset) override;
    bool endOfStream() const override;

    size_t read(void* out, size_t size) override;
    size_t write(const void* in, size_t size) override;

    uint64_t getLength() override;
};

class BufferDataStream : public DataStream {
private:
    uint8_t* buffer = nullptr;
    uint64_t readIndex = 0;
    uint64_t writeIndex = 0;
    uint64_t length = 0;
public:
    BufferDataStream(const uint8_t* buffer, uint64_t length);
    BufferDataStream(uint64_t length);
    ~BufferDataStream() override;

    uint64_t getReadIndex() override;
    uint64_t getWriteIndex() override;

    void peekBytes(void* out, uint64_t pos, size_t size);
    void seekRead(uint64_t pos) override;
    void skipRead(int64_t offset) override;
    void seekWrite(uint64_t pos) override;
    void skipWrite(int64_t offset) override;
    bool endOfStream() const override;

    size_t read(void* out, size_t size) override;
    size_t write(const void* in, size_t size) override;

    uint64_t getLength() override;
};

class DataSerializer {
protected:
    std::shared_ptr<DataStream> stream;
    uint64_t chunkSize;
public:
    DataSerializer(std::shared_ptr<DataStream> stream, uint64_t chunkSize = 4096);

    uint64_t getLength();
    uint64_t getWritePos();

    void write(const void* bytes, size_t size);

    void skip(uint64_t offset);
    void rewind(uint64_t offset_inv);
};

class DataDeserializer {
protected:
    std::shared_ptr<DataStream> stream;
    uint64_t chunkSize;
public:
    DataDeserializer(std::shared_ptr<DataStream> stream, uint64_t chunkSize = 4096);

    uint64_t getLength();
    uint64_t getReadPos();

    uint8_t readByte();
    void read(void* out, size_t size);

    void skip(uint64_t offset);
    void rewind(uint64_t offset_inv);
};

void writeBufferStreamToFile(std::shared_ptr<BufferDataStream> stream, uint64_t pos, uint64_t size, const std::filesystem::path& path, uint64_t chunkSize = 4096);
void writeBufferStreamToFile(std::shared_ptr<BufferDataStream> stream, uint64_t size, const std::filesystem::path& path, uint64_t chunkSize = 4096);
void writeBufferStreamToFile(std::shared_ptr<BufferDataStream> stream, const std::filesystem::path& path, uint64_t chunkSize = 4096);

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