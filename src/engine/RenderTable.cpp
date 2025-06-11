#include <Syngine/engine/RenderTable.hpp>
#include <algorithm>

template class RenderTable<WindowRenderable>;
template class RenderTable<ShaderRenderable>;

template void RenderTable<WindowRenderable>::add(const std::string&, WindowRenderable*);
template void RenderTable<ShaderRenderable>::add(const std::string&, ShaderRenderable*);

template void RenderTable<WindowRenderable>::remove(const std::string&);
template void RenderTable<ShaderRenderable>::remove(const std::string&);

template WindowRenderable* RenderTable<WindowRenderable>::get(const std::string&) const;
template ShaderRenderable* RenderTable<ShaderRenderable>::get(const std::string&) const;

template void RenderTable<WindowRenderable>::forEach(const std::function<void(const std::string&, WindowRenderable*)>&) const;
template void RenderTable<ShaderRenderable>::forEach(const std::function<void(const std::string&, ShaderRenderable*)>&) const;

template <typename R>
void RenderTable<R>::add(const std::string& key, R* renderable) {
    if (objects.find(key) == objects.end()) {
        insertionOrder.push_back(key);
    }
    objects[key] = renderable;
}

template <typename R>
void RenderTable<R>::remove(const std::string& key) {
    if (objects.erase(key)) {
        insertionOrder.erase(
            std::remove(insertionOrder.begin(), insertionOrder.end(), key),
            insertionOrder.end()
        );
    }
}

template <typename R>
R* RenderTable<R>::get(const std::string& key) const {
    auto it = objects.find(key);
    return it != objects.end() ? it->second : nullptr;
}

template <typename R>
void RenderTable<R>::forEach(const std::function<void(const std::string&, R*)>& func) const {
    for (const auto& key : insertionOrder) {
        auto it = objects.find(key);
        if (it != objects.end()) {
            func(key, it->second);
        }
    }
}