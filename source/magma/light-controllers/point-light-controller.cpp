#include <lava/magma/light-controllers/point-light-controller.hpp>

#include <lava/magma/light.hpp>

using namespace lava::magma;

void PointLightController::bind(Light& light)
{
    m_light = &light;

    // Force UBO update default values.
    auto& ubo = m_light->ubo();
    ubo.type = static_cast<uint32_t>(LightType::Point);
    ubo.data[0].x = reinterpret_cast<const uint32_t&>(m_translation.x);
    ubo.data[0].y = reinterpret_cast<const uint32_t&>(m_translation.y);
    ubo.data[0].z = reinterpret_cast<const uint32_t&>(m_translation.z);
    ubo.data[1].x = reinterpret_cast<const uint32_t&>(m_radius);

    m_light->uboChanged();
}

// ----- Controls

void PointLightController::translation(const glm::vec3& translation)
{
    m_translation = translation;

    auto& ubo = m_light->ubo();
    ubo.data[0].x = reinterpret_cast<const uint32_t&>(m_translation.x);
    ubo.data[0].y = reinterpret_cast<const uint32_t&>(m_translation.y);
    ubo.data[0].z = reinterpret_cast<const uint32_t&>(m_translation.z);

    m_light->uboChanged();
}

void PointLightController::radius(float radius)
{
    m_radius = radius;

    auto& ubo = m_light->ubo();
    ubo.data[1].x = reinterpret_cast<const uint32_t&>(m_radius);

    m_light->uboChanged();
}
