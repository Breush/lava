#pragma once

namespace {
    template <class T>
    struct RenderEngineMaker {
        template <class... Arguments>
        static T& make(lava::magma::RenderEngine& engine, Arguments&&... arguments)
        {
            auto pResource = std::make_unique<T>(engine, std::forward<Arguments>(arguments)...);
            auto& resource = *pResource;
            engine.add(std::move(pResource));
            return resource;
        }
    };

    template <>
    struct RenderEngineMaker<lava::magma::Scene> {
        template <class... Arguments>
        static inline lava::magma::Scene& make(lava::magma::RenderEngine& engine, Arguments&&... arguments)
        {
            auto resource = engine.sceneAllocator().allocate<lava::magma::Scene>(engine, std::forward<Arguments>(arguments)...);
            engine.add(*resource);
            return *resource;
        }
    };
}

namespace lava::magma {
    template <class T, class... Arguments>
    inline T& RenderEngine::make(Arguments&&... arguments)
    {
        return RenderEngineMaker<T>::make(*this, std::forward<Arguments>(arguments)...);
    }
}
