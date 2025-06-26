#pragma once

#include "Syngine/modules/Shader.hpp"
#include <unordered_map>
#include <string>
#include <functional>

class GameWindow;

class WindowRenderable
{
public:
    virtual void render(GameWindow* window, int parentFBO = 0) = 0;

    virtual ~WindowRenderable() = default;
};

class ShaderRenderable
{
public:
    virtual void render(Shader shader, int parentFBO = 0) = 0;

    virtual ~ShaderRenderable() = default;
};

class DuplexRenderable : public ShaderRenderable, public WindowRenderable {
public:
    virtual ~DuplexRenderable() = default;
};

template<typename R>
class RenderTable {
    static_assert(
        std::is_base_of<ShaderRenderable, R>::value ||
        std::is_base_of<WindowRenderable, R>::value,
        "ERROR::ASSERT<RenderableType must inherit from ShaderRenderable, WindowRenderable or FramebufferRenderable>"
    );

    std::unordered_map<std::string, R*> objects;
    std::vector<std::string> insertionOrder;
public:
    void add(const std::string& key, R* renderable);

    void wipe(const std::string& key);

    R* remove(const std::string& key);

    R* get(const std::string& key) const;

    void forEach(const std::function<void(const std::string&, R*)>& func) const;
};