#pragma once

namespace lava::flow {
    template <class T, class... Arguments>
    inline T& AudioEngine::make(Arguments&&... arguments)
    {
        auto pResource = std::make_unique<T>(*this, std::forward<Arguments>(arguments)...);
        auto& resource = *pResource;
        add(std::move(pResource));
        return resource;
    }

    template <class T, class... Arguments>
    inline std::shared_ptr<T> AudioEngine::share(Arguments&&... arguments)
    {
        return std::make_shared<T>(std::forward<Arguments>(arguments)...);
    }
}
