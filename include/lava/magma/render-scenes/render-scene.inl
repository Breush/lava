#pragma once

namespace lava::magma {
    template <class T, class... Arguments>
    inline T& RenderScene::make(Arguments&&... arguments)
    {
        auto pResource = std::make_unique<T>(*this, std::forward<Arguments>(arguments)...);
        auto& resource = *pResource;
        add(std::move(pResource));
        return resource;
    }

    template <class T, class... Arguments>
    inline T& RenderScene::make(std::function<void(T&)> maker, Arguments&&... arguments)
    {
        auto& resource = make<T>(std::forward<Arguments>(arguments)...);
        maker(resource);
        return resource;
    }
}
