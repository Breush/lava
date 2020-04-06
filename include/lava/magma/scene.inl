#pragma once

namespace lava::magma {
    template <class T>
    struct SceneResourceMaker {
        template <class... Arguments>
        static T& make(Scene& scene, Arguments&&... arguments) = delete;
    };

    template <>
    struct SceneResourceMaker<Light> {
        template <class... Arguments>
        static inline Light& make(Scene& scene, Arguments&&... arguments)
        {
            return scene.makeLight(std::forward<Arguments>(arguments)...);
        }
    };

    template <>
    struct SceneResourceMaker<Camera> {
        template <class... Arguments>
        static inline Camera& make(Scene& scene, Arguments&&... arguments)
        {
            return scene.makeCamera(std::forward<Arguments>(arguments)...);
        }
    };

    template <>
    struct SceneResourceMaker<Material> {
        template <class... Arguments>
        static inline Material& make(Scene& scene, Arguments&&... arguments)
        {
            return scene.makeMaterial(std::forward<Arguments>(arguments)...);
        }
    };

    template <>
    struct SceneResourceMaker<Texture> {
        template <class... Arguments>
        static inline Texture& make(Scene& scene, Arguments&&... arguments)
        {
            return scene.makeTexture(std::forward<Arguments>(arguments)...);
        }
    };

    template <>
    struct SceneResourceMaker<Mesh> {
        template <class... Arguments>
        static inline Mesh& make(Scene& scene, Arguments&&... arguments)
        {
            return scene.makeMesh(std::forward<Arguments>(arguments)...);
        }
    };

    template <>
    struct SceneResourceMaker<Flat> {
        template <class... Arguments>
        static inline Flat& make(Scene& scene, Arguments&&... arguments)
        {
            return scene.makeFlat(std::forward<Arguments>(arguments)...);
        }
    };

    template <class T, class... Arguments>
    inline T& Scene::make(Arguments&&... arguments)
    {
        return SceneResourceMaker<T>::make(*this, std::forward<Arguments>(arguments)...);
    }
}
