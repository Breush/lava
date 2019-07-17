#pragma once

namespace lava::magma {
    class Light;
}

namespace lava::magma {
    /**
     * Controls the light so that it renders as a point one.
     */
    class PointLightController {
    public:
        PointLightController() {}
        PointLightController(Light& light) { bind(light); }

        /// Bind a light to this controller.
        void bind(Light& light);

        /**
         * @name Controls
         */
        /// @{
        /// The position of the light.
        const glm::vec3& translation() const { return m_translation; }
        void translation(const glm::vec3& translation);

        /// The distance at which the intensity of the light goes first to 0.
        float radius() const { return m_radius; }
        void radius(float radius);
        /// @}

    private:
        Light* m_light;

        // ----- Controls
        glm::vec3 m_translation = glm::vec3(0.f);
        float m_radius = 1.f;
    };
}
