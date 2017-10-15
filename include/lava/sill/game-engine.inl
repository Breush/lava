#pragma once

namespace lava::sill {
    template <class T, class... Arguments>
    inline T& GameEngine::make(Arguments&&... arguments)
    {
        auto pResource = std::make_unique<T>(*this, std::forward<Arguments>(arguments)...);
        auto& resource = *pResource;
        add(std::move(pResource));
        return resource;
    }
}
