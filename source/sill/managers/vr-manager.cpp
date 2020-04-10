#include <lava/sill/managers/vr-manager.hpp>

#include <lava/sill/game-engine.hpp>

using namespace lava::sill;

VrManager::VrManager(GameEngine& engine)
    : m_engine(engine)
{
}

const glm::vec3& VrManager::translation() const
{
    return m_engine.renderEngine().vrTranslation();
}

void VrManager::translation(const glm::vec3& translation)
{
    m_engine.renderEngine().vrTranslation(translation);
}

bool VrManager::enabled() const
{
    return m_engine.renderEngine().vrEnabled();
}

bool VrManager::deviceValid(VrDeviceType deviceType) const
{
    return m_engine.renderEngine().vrDeviceValid(deviceType);
}

const glm::mat4& VrManager::deviceTransform(VrDeviceType deviceType) const
{
    return m_engine.renderEngine().vrDeviceTransform(deviceType);
}
