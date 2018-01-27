#pragma once

namespace lava::dike {
    template <class T, class... Arguments>
    inline T& PhysicsEngine::make(Arguments&&... arguments)
    {
        auto pResource = std::make_unique<T>(*this, std::forward<Arguments>(arguments)...);
        auto& resource = *pResource;
        add(std::move(pResource));
        return resource;
    }
}
