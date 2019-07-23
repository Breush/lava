#include <lava/magma/light-controllers/directional-light-controller.hpp>

#include <lava/magma/light.hpp>

using namespace lava::magma;

void DirectionalLightController::bind(Light& light)
{
    m_light = &light;

    m_light->ubo().type = static_cast<uint32_t>(LightType::Directional);

    m_light->shadowsEnabled(true);

    // Force UBO update with default value.
    direction(m_direction);
}

// ----- Controls

void DirectionalLightController::direction(const glm::vec3& direction)
{
    m_direction = glm::normalize(direction);

    auto& ubo = m_light->ubo();
    ubo.data[0].x = reinterpret_cast<const uint32_t&>(m_direction.x);
    ubo.data[0].y = reinterpret_cast<const uint32_t&>(m_direction.y);
    ubo.data[0].z = reinterpret_cast<const uint32_t&>(m_direction.z);

    m_light->shadowsDirection(m_direction);
    m_light->uboChanged();
}
