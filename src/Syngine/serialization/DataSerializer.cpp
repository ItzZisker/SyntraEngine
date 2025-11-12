#include "DataSerializer.hpp"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace syng;

FileDataStream::FileDataStream(const std::filesystem::path& path, bool read, bool write) {
    std::ios::openmode mode = std::ios::binary;
    if (read)  mode |= std::ios::in;
    if (write) mode |= std::ios::out | std::ios::trunc;
    file.open(path, mode);
    if (!file.is_open()) throw std::runtime_error("Failed to open file: " + path.string());
    readable = read;
    writable = write;
}

FileDataStream::~FileDataStream() {
    file.close();
}

uint64_t FileDataStream::tell() {
    return static_cast<uint64_t>(file.tellg());
}

void FileDataStream::seek(uint64_t pos) {
    file.seekg(pos);
    file.seekp(pos);
}

void FileDataStream::skip(int64_t offset) {
    file.seekg(offset, std::ios::cur);
    file.seekp(offset, std::ios::cur);
}

bool FileDataStream::eof() const {
    return file.eof();
}

size_t FileDataStream::read(void* out, size_t size) {
    if (!readable) throw std::runtime_error("Stream not open for reading");
    file.read(reinterpret_cast<char*>(out), size);
    return static_cast<size_t>(file.gcount());
}

size_t FileDataStream::write(const void* in, size_t size) {
    if (!writable) throw std::runtime_error("Stream not open for writing");
    file.write(reinterpret_cast<const char*>(in), size);
    return size;
}

uint64_t FileDataStream::getLength() {
    if (cachedLength) return cachedLength;
    std::streampos current = file.tellg();
    file.seekg(0, std::ios::end);
    std::streampos end = file.tellg();
    file.seekg(current); // restore previous position
    return (this->cachedLength = static_cast<uint64_t>(end));
}

BufferDataStream::BufferDataStream(const uint8_t* buffer, uint64_t length) : length(length) {
    this->buffer = new uint8_t[length];
    if (buffer) std::memcpy(this->buffer, buffer, this->length);
}

BufferDataStream::BufferDataStream(uint64_t length) : length(length) {
    this->buffer = new uint8_t[length];
}

BufferDataStream::~BufferDataStream() {
    delete this->buffer;
    readIndex = writeIndex = length = 0;
}

uint64_t BufferDataStream::tell() {
    return this->readIndex;
}

class BufferDataStream : public DataStream {
private:
    const uint8_t* buffer;
    uint64_t readIndex = 0;
    uint64_t writeIndex = 0;
    uint64_t length;
public:
    uint64_t tell() override;
    void seek(uint64_t pos) override;
    void skip(int64_t offset) override;
    bool eof() const override;
    size_t read(void* out, size_t size) override;
    size_t write(const void* in, size_t size) override;
    uint64_t getLength() override;
};

DataSerializer::DataSerializer(uint64_t length) : data(new uint8_t[length]()), writeIndex(0), length(length) {}
DataSerializer::~DataSerializer() {
    delete[] data;
    this->length = 0;
    this->writeIndex = 0;
}

void DataSerializer::write(const unsigned char* bytes, size_t size) {
    if (writeIndex + size > length) {
        throw std::overflow_error("DataSerializer overflow");
    }
    std::memcpy(data + writeIndex, bytes, size);
    writeIndex += size;
}

void DataSerializer::rewind(uint64_t pos) {
    if (pos < 0) throw std::out_of_range("Negative rewind position");
    if (pos > writeIndex) throw std::out_of_range("Cannot rewind past current write position");
    std::memset(data + pos, 0, writeIndex - pos);
    writeIndex = pos;
}

void DataSerializer::serialize(std::filesystem::path path) {
    std::ofstream packfile;
    packfile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    packfile.open(path, std::ios::binary);
    packfile.write(reinterpret_cast<const char*>(copyData(writeIndex + 1).data()), writeIndex + 1);
    packfile.close();
}

DataDeserializer::DataDeserializer(const uint8_t* buffer, uint64_t length) : length(length) {
    this->data = new uint8_t[length];
    if (buffer) std::memcpy(this->data, buffer, this->length);
}

DataDeserializer::DataDeserializer(std::filesystem::path path, uint64_t length) {
    std::ifstream packfile(path, std::ios::binary | std::ios::ate);
    if (!packfile) {
        throw std::runtime_error("Failed to open file: " + path.string());
    }
    std::streamsize ssize = packfile.tellg();
    packfile.seekg(0, std::ios::beg);

    this->length = length == 0 ? ssize : std::min(length, (uint64_t) ssize);
    std::vector<uint8_t> bytes(this->length);
    if (!packfile.read(reinterpret_cast<char*>(bytes.data()), this->length)) {
        throw std::runtime_error("Failed to read file: " + path.string());
    }
    this->data = new uint8_t[this->length];
    std::memcpy(this->data, bytes.data(), this->length);
}

DataDeserializer::~DataDeserializer() {
    delete[] data;
    this->length = 0;
    this->readIndex = 0;
}

uint8_t DataDeserializer::readByte() {
    uint8_t bytes[1];
    read(bytes, 1);
    return bytes[0];
}

void DataDeserializer::read(unsigned char* outBytes, size_t size) {
    if (readIndex + size > length) throw std::out_of_range("DataDeserializer overflow");
    std::memcpy(outBytes, data + readIndex, size);
    readIndex += size;
}

void DataDeserializer::rewind(uint64_t pos) {
    if (pos < 0) throw std::out_of_range("Negative rewind position");
    if (pos > readIndex) throw std::out_of_range("Cannot rewind past current read position");
    readIndex = pos;
}

void DataDeserializer::skip(uint64_t size) {
    if (size < 0) throw std::out_of_range("Negative size");
    if (readIndex + size > length) throw std::out_of_range("Cannot skip past buffer end");
    readIndex += size;
}