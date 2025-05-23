#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <functional>

namespace kcomp
{
    class Renderable;
}

class RenderTable {
    std::unordered_map<std::string, kcomp::Renderable*> objects;

public:
    void add(const std::string& key, kcomp::Renderable* renderable);

    void remove(const std::string& key);

    kcomp::Renderable* get(const std::string& key) const;

    void forEach(const std::function<void(const std::string&, kcomp::Renderable*)>& func) const;
};
