#pragma once

#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/modules/Shader.hpp"

#include <unordered_map>
#include <functional>
#include <algorithm>
#include <string>

namespace syng
{
class GameWindow;
class MeshInstance;
class Mesh;

bool RT_SORT_OPACITY_SUB_MESH(Mesh* a, Mesh* b);
bool RT_SORT_OPACITY_GROUP_MESH(MeshInstance* a, MeshInstance* b);

class WindowRenderable
{
public:
    virtual void render(GameWindow* window) = 0;
    virtual ~WindowRenderable() = default;
};

class ShaderRenderable
{
public:
    virtual void render(Shader& shader, Screenbuffer screen = {}) = 0;
    virtual ~ShaderRenderable() = default;
};

class DuplexRenderable : public ShaderRenderable, public WindowRenderable {
public:
    virtual ~DuplexRenderable() = default;
};

template<typename R>
class RenderTable {
    std::unordered_map<std::string, R*> objects;
    std::vector<std::string> insertionOrder;
public:
    void addAll(std::unordered_map<std::string, R*> map) {
        for (auto& pair : map) {
            add(pair.first, pair.second);
        }
    }

    void add(const std::string& key, R* renderable) {
        if (objects.find(key) == objects.end()) {
            insertionOrder.push_back(key);
        }
        objects[key] = renderable;
    }

    void add(RenderTable<R>* rendertable) {
        rendertable->forEach([&](const std::string& key, R* renderable) {
            add(key, renderable);
        });
    }

    void wipe(const std::string& key) {
        R* r = remove(key);
        if (r) delete r;
    }

    void wipeAll() {
        for (const auto& pair : objects) wipe(pair.first);
        clear();
    }

    void clear() {
        objects.clear();
        insertionOrder.clear();
    }

    R* remove(const std::string& key) {
        auto it = objects.find(key);
        if (it != objects.end()) {
            R* r = it->second;
            objects.erase(it);
            insertionOrder.erase(
                std::remove(insertionOrder.begin(), insertionOrder.end(), key),
                insertionOrder.end()
            );
            return r;
        }
        return nullptr;
    }

    R* get(const std::string& key) const {
        auto it = objects.find(key);
        return it != objects.end() ? it->second : nullptr;
    }

    void sort(const std::function<bool(R* a, R* b)>& compare) {
        std::sort(insertionOrder.begin(), insertionOrder.end(),
            [this, &compare](const std::string& a, const std::string& b) {
                auto itA = objects.find(a);
                auto itB = objects.find(b);
                if (itA == objects.end() || itB == objects.end()) {
                    return false;
                }
                return compare(itA->second, itB->second);
            }
        );
    }

    void forEach(const std::function<void(const std::string&, R*)>& func) const {
        for (const auto& key : insertionOrder) {
            auto it = objects.find(key);
            if (it != objects.end()) {
                func(key, it->second);
            }
        }
    }

    std::unordered_map<std::string, R*> asMap() {
        return this->objects;
    }
};
}