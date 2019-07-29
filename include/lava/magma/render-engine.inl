#pragma once

namespace lava::magma {
    template <class T>
    struct RenderEngineResourceMaker {
        template <class... Arguments>
        static T& make(RenderEngine& engine, Arguments&&... arguments)
        {
            auto pResource = std::make_unique<T>(engine, std::forward<Arguments>(arguments)...);
            auto& resource = *pResource;
            engine.add(std::move(pResource));
            return resource;
        }
    };

    template <>
    struct RenderEngineResourceMaker<Scene> {
        template <class... Arguments>
        static inline Scene& make(RenderEngine& engine, Arguments&&... arguments)
        {
            return engine.makeScene(std::forward<Arguments>(arguments)...);
        }
    };

    template <class T, class... Arguments>
    inline T& RenderEngine::make(Arguments&&... arguments)
    {
        return RenderEngineResourceMaker<T>::make(*this, std::forward<Arguments>(arguments)...);
    }
}
