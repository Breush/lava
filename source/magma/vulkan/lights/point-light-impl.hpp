#pragma once

#include <lava/magma/lights/point-light.hpp>

#include "./i-light-impl.hpp"

#include <lava/core/macros.hpp>
#include <lava/magma/render-scenes/render-scene.hpp>

namespace lava::magma {
    /**
     * Implementation of magma::PointLight.
     */
    class PointLight::Impl : public ILight::Impl {
    public:
        Impl(RenderScene& scene);
        ~Impl() = default;

        // ILight::Impl
        void init(uint32_t id) override final;
        LightType type() const override final { return LightType::Point; };

        void renderShadows(vk::CommandBuffer /*commandBuffer*/, vk::PipelineLayout /*pipelineLayout*/,
                           uint32_t /*descriptorSetIndex*/) const override final
        {
            // @todo Make it do something?
        }

    private:
        // References
        RenderScene::Impl& m_scene;

        // ILight
        $property(glm::vec3, translation);
        $property(float, radius, = 1.f);
    };
}
