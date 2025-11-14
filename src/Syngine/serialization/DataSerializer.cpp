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

uint64_t FileDataStream::getReadIndex() {
    return readable ? static_cast<uint64_t>(file.tellg()) : -1;
}

uint64_t FileDataStream::getWriteIndex() {
    return writable ? static_cast<uint64_t>(file.tellp()) : -1;
}

void FileDataStream::seekRead(uint64_t pos) {
    if (!readable) throw std::runtime_error("Stream not open for reading");
    file.seekg(pos);
}

void FileDataStream::seekWrite(uint64_t pos) {
    if (!writable) throw std::runtime_error("Stream not open for writing");
    file.seekp(pos);
}

void FileDataStream::skipRead(int64_t offset) {
    if (!readable) throw std::runtime_error("Stream not open for reading");
    file.seekg(offset, std::ios::cur);
}

void FileDataStream::skipWrite(int64_t offset) {
    if (!writable) throw std::runtime_error("Stream not open for writing");
    file.seekp(offset, std::ios::cur);
}

bool FileDataStream::endOfStream() const {
    return !readable || file.eof();
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

uint64_t BufferDataStream::getReadIndex() {
    return this->readIndex;
}

uint64_t BufferDataStream::getWriteIndex() {
    return this->writeIndex;
}

void BufferDataStream::seekRead(uint64_t pos) {
    this->readIndex = pos;
}

void BufferDataStream::skipRead(int64_t offset) {
    this->readIndex += offset;
}

void BufferDataStream::seekWrite(uint64_t pos) {
    this->writeIndex = pos;
}

void BufferDataStream::skipWrite(int64_t offset) {
    this->writeIndex += offset;
}

bool BufferDataStream::endOfStream() const {
    return this->readIndex >= this->length;
}

size_t BufferDataStream::read(void* out, size_t size) {
    size = std::min(size, this->length - this->readIndex);
    std::memcpy(out, this->buffer + this->readIndex, size);
    this->readIndex += size;
    return size;
}

size_t BufferDataStream::write(const void* in, size_t size) {
    size = std::min(size, this->length - this->writeIndex);
    std::memcpy(this->buffer + this->writeIndex, in, size);
    this->writeIndex += size;
    return size;
}

void BufferDataStream::peekBytes(void* out, uint64_t pos, size_t size) {
    pos = std::clamp(pos, uint64_t(0), this->length);
    size = std::min(size, this->length - pos);
    std::memcpy(out, this->buffer + pos, size);
}

uint64_t BufferDataStream::getLength() {
    return this->length;
}

DataSerializer::DataSerializer(std::shared_ptr<DataStream> stream, uint64_t chunkSize)
    : stream(stream), chunkSize(chunkSize) {}

uint64_t DataSerializer::getLength() {
    return stream->getLength();
}

uint64_t DataSerializer::getWritePos() {
    return stream->getWriteIndex();
}

void DataSerializer::write(const void* bytes, size_t size) {
    stream->write(bytes, size);
}

void DataSerializer::skip(uint64_t pos) {
    stream->seekWrite(pos);
}

void DataSerializer::rewind(uint64_t pos) {
    stream->seekWrite(-pos);
}

DataDeserializer::DataDeserializer(std::shared_ptr<DataStream> stream, uint64_t chunkSize)
    : stream(stream), chunkSize(chunkSize) {}

uint64_t DataDeserializer::getLength() {
    return stream->getLength();
}

uint64_t DataDeserializer::getReadPos() {
    return stream->getReadIndex();
}

uint8_t DataDeserializer::readByte() {
    uint8_t byte;
    stream->read(&byte, 1);
    return byte;
}

void DataDeserializer::read(void* out, size_t size) {
    stream->read(out, size);
}

void DataDeserializer::rewind(uint64_t offset_inv) {
    stream->seekRead(-offset_inv);
}

void DataDeserializer::skip(uint64_t offset) {
    stream->skipRead(offset);
}

void syng::writeBufferStreamToFile(std::shared_ptr<BufferDataStream> stream, uint64_t pos, uint64_t size, const std::filesystem::path &path, uint64_t chunkSize) {
    pos = std::clamp(pos, uint64_t(0), stream->getLength());
    size = std::min(size, stream->getLength() - pos);
    uint64_t *chunk = new uint64_t[chunkSize];
    uint64_t start = 0, end = 0;
    for (uint64_t i = 0; i < size; i += chunkSize) {
        end = std::min(i + chunkSize, size);
        stream->peekBytes(chunk, pos + i, end - i);
        std::ofstream packfile;
        packfile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        packfile.open(path, std::ios::binary | std::ios::app);
        packfile.write(reinterpret_cast<const char*>(chunk), end - i);
        packfile.close();
    }
    delete[] chunk;
}

void syng::writeBufferStreamToFile(std::shared_ptr<BufferDataStream> stream, uint64_t size, const std::filesystem::path &path, uint64_t chunkSize) {
    writeBufferStreamToFile(stream, 0, size, path, chunkSize);
}

void syng::writeBufferStreamToFile(std::shared_ptr<BufferDataStream> stream, const std::filesystem::path &path, uint64_t chunkSize) {
    writeBufferStreamToFile(stream, 0, stream->getLength(), path, chunkSize);
}