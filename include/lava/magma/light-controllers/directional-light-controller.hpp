#pragma once

namespace lava::magma {
    class Light;
}

namespace lava::magma {
    /**
     * Controls the light so that it renders as a directional one.
     */
    class DirectionalLightController {
    public:
        DirectionalLightController() {}
        DirectionalLightController(Light& light) { bind(light); }

        /// Bind a light to this controller.
        void bind(Light& light);

        /**
         * @name Controls
         */
        /// @{
        /// The direction of lighting, will always be normalized.
        const glm::vec3& direction() const { return m_direction; }
        void direction(const glm::vec3& direction);
        /// @}

    private:
        Light* m_light;

        // ----- Controls
        glm::vec3 m_direction = glm::vec3(1, 0, 0);
    };
}
