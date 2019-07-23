#pragma once

namespace {
    template <class T>
    struct SceneMaker {
        template <class... Arguments>
        static T& make(lava::magma::Scene& scene, Arguments&&... arguments);
    };

    template <>
    struct SceneMaker<lava::magma::Light> {
        template <class... Arguments>
        static inline lava::magma::Light& make(lava::magma::Scene& scene, Arguments&&... arguments)
        {
            auto resource = scene.cameraAllocator().allocate<lava::magma::Light>(scene, std::forward<Arguments>(arguments)...);
            scene.add(*resource);
            return *resource;
        }
    };

    template <>
    struct SceneMaker<lava::magma::Camera> {
        template <class... Arguments>
        static inline lava::magma::Camera& make(lava::magma::Scene& scene, Arguments&&... arguments)
        {
            auto resource = scene.cameraAllocator().allocate<lava::magma::Camera>(scene, std::forward<Arguments>(arguments)...);
            scene.add(*resource);
            return *resource;
        }
    };

    template <>
    struct SceneMaker<lava::magma::Material> {
        template <class... Arguments>
        static inline lava::magma::Material& make(lava::magma::Scene& scene, Arguments&&... arguments)
        {
            auto resource =
                scene.materialAllocator().allocate<lava::magma::Material>(scene, std::forward<Arguments>(arguments)...);
            scene.add(*resource);
            return *resource;
        }
    };

    template <>
    struct SceneMaker<lava::magma::Texture> {
        template <class... Arguments>
        static inline lava::magma::Texture& make(lava::magma::Scene& scene, Arguments&&... arguments)
        {
            auto resource = scene.textureAllocator().allocate<lava::magma::Texture>(scene, std::forward<Arguments>(arguments)...);
            scene.add(*resource);
            return *resource;
        }
    };

    template <>
    struct SceneMaker<lava::magma::Mesh> {
        template <class... Arguments>
        static inline lava::magma::Mesh& make(lava::magma::Scene& scene, Arguments&&... arguments)
        {
            auto resource = scene.meshAllocator().allocate<lava::magma::Mesh>(scene, std::forward<Arguments>(arguments)...);
            scene.add(*resource);
            return *resource;
        }
    };
}

namespace lava::magma {
    template <class T, class... Arguments>
    inline T& Scene::make(Arguments&&... arguments)
    {
        return SceneMaker<T>::make(*this, std::forward<Arguments>(arguments)...);
    }
}
