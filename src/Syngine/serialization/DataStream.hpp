#pragma once

#include "DataSerializer.hpp"

#include <cstdint>
#include <vector>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>

using namespace syng;

namespace syng
{


class FileDataStream : public DataStream {
private:
    std::fstream file;
    bool readable = false;
    bool writable = false;
    uint64_t cachedLength = 0;
public:
    FileDataStream(const std::filesystem::path& path, bool read, bool write) {
        std::ios::openmode mode = std::ios::binary;
        if (read)  mode |= std::ios::in;
        if (write) mode |= std::ios::out | std::ios::trunc;
        file.open(path, mode);
        if (!file.is_open()) throw std::runtime_error("Failed to open file: " + path.string());
        readable = read;
        writable = write;
    }

    ~FileDataStream() override {
        file.close();
    }

    uint64_t tell() override {
        return static_cast<uint64_t>(file.tellg());
    }

    void seek(uint64_t pos) override {
        file.seekg(pos);
        file.seekp(pos);
    }

    void skip(int64_t offset) override {
        file.seekg(offset, std::ios::cur);
        file.seekp(offset, std::ios::cur);
    }

    bool eof() const override {
        return file.eof();
    }

    size_t read(void* out, size_t size) override {
        if (!readable) throw std::runtime_error("Stream not open for reading");
        file.read(reinterpret_cast<char*>(out), size);
        return static_cast<size_t>(file.gcount());
    }

    size_t write(const void* in, size_t size) override {
        if (!writable) throw std::runtime_error("Stream not open for writing");
        file.write(reinterpret_cast<const char*>(in), size);
        return size;
    }

    uint64_t getLength() override {
        if (cachedLength) return cachedLength;
        std::streampos current = file.tellg();
        file.seekg(0, std::ios::end);
        std::streampos end = file.tellg();
        file.seekg(current); // restore previous position
        return (this->cachedLength = static_cast<uint64_t>(end));
    }
};

class DataSerializerStream : public DataSerializer {
private:
    std::shared_ptr<DataStream> stream;
    uint64_t chunkSize;

public:
    DataSerializerStream(std::shared_ptr<DataStream> stream, uint64_t chunkSize = 4096)
        : DataSerializer(chunkSize), stream(std::move(stream)), chunkSize(chunkSize)
    {}

    void write(const unsigned char* bytes, size_t size) override {
        size_t totalWritten = 0;
        while (totalWritten < size) {
            size_t toWrite = std::min<size_t>(chunkSize, size - totalWritten);
            stream->write(bytes + totalWritten, toWrite);
            totalWritten += toWrite;
            writeIndex += toWrite;
        }
    }

    void rewind(uint64_t pos) override {
        stream->seek(pos);
        writeIndex = pos;
    }

    void serialize(std::filesystem::path) override {}
};

class DataDeserializerStream : public DataDeserializer {
private:
    std::shared_ptr<DataStream> stream;
    uint64_t chunkSize;

public:
    DataDeserializerStream(std::shared_ptr<DataStream> stream, uint64_t chunkSize = 4096)
        : DataDeserializer(nullptr, 0), stream(std::move(stream)), chunkSize(chunkSize)
    {}

    uint8_t readByte() override {
        uint8_t b;
        if (stream->read(&b, 1) != 1)
            throw std::runtime_error("End of stream");
        readIndex++;
        return b;
    }

    void read(unsigned char* outBytes, size_t size) override {
        size_t totalRead = 0;
        while (totalRead < size) {
            size_t toRead = std::min<size_t>(chunkSize, size - totalRead);
            size_t bytesRead = stream->read(outBytes + totalRead, toRead);
            if (bytesRead == 0) throw std::runtime_error("End of stream");
            totalRead += bytesRead;
            readIndex += bytesRead;
        }
    }

    void rewind(uint64_t pos) override {
        stream->seek(pos);
        readIndex = pos;
    }

    void skip(uint64_t size) override {
        stream->skip(size);
        readIndex += size;
    }
};

}