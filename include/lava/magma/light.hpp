#pragma once

#include <lava/core/macros/aft.hpp>
#include <lava/magma/render-image.hpp>
#include <lava/magma/ubos.hpp>

namespace lava::magma {
    class LightAft;
    class Scene;
}

namespace lava::magma {
    /**
     * A generic light.
     *
     * It is expected to get modified through
     * `DirectionalLightController` or such.
     *
     * ```c++
     * auto& light = scene.make<Light>();
     * DirectionalLightController lightController(light);
     * ```
     */
    class Light {
    public:
        Light(Scene& scene);
        ~Light();

        $aft_class(Light);

        Scene& scene() { return m_scene; }
        const Scene& scene() const { return m_scene; }

        /**
         * @name Rendering
         */
        /// @{
        /// The shadow map rendered image (if any).
        RenderImage shadowsRenderImage() const;

        /// What direction the light goes. The provided vector must be normalized.
        /// @note Note currently handling shadows for non-directional lights.
        const glm::vec3& shadowsDirection() const { return m_shadowsDirection; }
        void shadowsDirection(const glm::vec3& shadowsDirection) { m_shadowsDirection = shadowsDirection; }

        /// Whether the light should cast dynamic shadows.
        bool shadowsEnabled() const { return m_shadowsEnabled; }
        void shadowsEnabled(bool shadowsEnabled) { m_shadowsEnabled = shadowsEnabled; }
        /// @}

        /**
         * @name Shader data
         */
        /// @{
        LightUbo& ubo() { return m_ubo; }
        const LightUbo& ubo() const { return m_ubo; }

        void uboChanged();
        /// @}

    private:
        // ----- References
        Scene& m_scene;

        // ----- Shader data
        LightUbo m_ubo;

        // ----- Rendering
        glm::vec3 m_shadowsDirection = glm::vec3(1, 0, 0);
        bool m_shadowsEnabled = false;
    };
}
