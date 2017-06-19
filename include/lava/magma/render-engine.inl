#pragma once

namespace lava {
    template <class T>
    inline T& RenderEngine::make()
    {
        auto pResource = std::make_unique<T>();
        auto& resource = *pResource;
        add(std::move(pResource));
        return resource;
    }
}
