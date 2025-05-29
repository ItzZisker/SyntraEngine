#pragma once

#include <unordered_map>
#include <string>
#include <functional>

class GameWindow;

class Renderable
{
public:
    virtual void render(GameWindow* window) = 0;

    virtual ~Renderable() = default;
};

class RenderTable {
    std::unordered_map<std::string, Renderable*> objects;

public:
    void add(const std::string& key, Renderable* renderable);

    void remove(const std::string& key);

    Renderable* get(const std::string& key) const;

    void forEach(const std::function<void(const std::string&, Renderable*)>& func) const;
};
