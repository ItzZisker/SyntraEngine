#include <KEngine/engine/RenderTable.hpp>

void RenderTable::add(const std::string& key, Renderable* renderable) {
    objects[key] = renderable;
}

void RenderTable::remove(const std::string& key) {
    objects.erase(key);
}

Renderable* RenderTable::get(const std::string& key) const {
    auto it = objects.find(key);
    return (it != objects.end()) ? it->second : nullptr;
}

void RenderTable::forEach(const std::function<void(const std::string&, Renderable*)>& func) const {
    for (const auto& [key, renderable] : objects) {
        func(key, renderable);
    }
}