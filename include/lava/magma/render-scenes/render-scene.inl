#pragma once

namespace {
    template <class T>
    struct RenderSceneMaker {
        template <class... Arguments>
        static inline T& make(lava::magma::RenderScene& scene, Arguments&&... arguments)
        {
            auto pResource = std::make_unique<T>(scene, std::forward<Arguments>(arguments)...);
            auto& resource = *pResource;
            scene.add(std::move(pResource));
            return resource;
        }
    };

    template <>
    struct RenderSceneMaker<lava::magma::Mesh> {
        template <class... Arguments>
        static inline lava::magma::Mesh& make(lava::magma::RenderScene& scene, Arguments&&... arguments)
        {
            auto resource = scene.meshAllocator().allocate<lava::magma::Mesh>(scene, std::forward<Arguments>(arguments)...);
            scene.add(*resource);
            return *resource;
        }
    };
}

namespace lava::magma {
    template <class T, class... Arguments>
    inline T& RenderScene::make(Arguments&&... arguments)
    {
        return RenderSceneMaker<T>::make(*this, std::forward<Arguments>(arguments)...);
    }
}
