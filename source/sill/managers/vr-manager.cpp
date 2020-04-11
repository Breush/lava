#include <lava/sill/managers/vr-manager.hpp>

#include <lava/sill/game-engine.hpp>

using namespace lava::sill;

VrManager::VrManager(GameEngine& engine)
    : m_engine(engine)
{
}

void VrManager::update()
{
    auto& renderEngine = m_engine.renderEngine();
    if (!renderEngine.vr().enabled()) return;

    if (renderEngine.vr().deviceValid(VrDeviceType::LeftHand)) {
        auto& mesh = renderEngine.vr().deviceMesh(VrDeviceType::LeftHand, m_engine.scene());
        mesh.transform(renderEngine.vr().deviceTransform(VrDeviceType::LeftHand));
    }
    if (renderEngine.vr().deviceValid(VrDeviceType::RightHand)) {
        auto& mesh = renderEngine.vr().deviceMesh(VrDeviceType::RightHand, m_engine.scene());
        mesh.transform(renderEngine.vr().deviceTransform(VrDeviceType::RightHand));
    }
    if (renderEngine.vr().deviceValid(VrDeviceType::Head)) {
        auto& mesh = renderEngine.vr().deviceMesh(VrDeviceType::Head, m_engine.scene());
        mesh.transform(renderEngine.vr().deviceTransform(VrDeviceType::Head));
    }
}

bool VrManager::enabled() const
{
    return m_engine.renderEngine().vr().enabled();
}

bool VrManager::deviceValid(VrDeviceType deviceType) const
{
    return m_engine.renderEngine().vr().deviceValid(deviceType);
}

const glm::mat4& VrManager::deviceTransform(VrDeviceType deviceType) const
{
    return m_engine.renderEngine().vr().deviceTransform(deviceType);
}

void VrManager::translation(const glm::vec3& translation)
{
    m_tranlation = translation;

    // @todo Currently only handling translation for the area transform!
    auto transform = glm::translate(glm::mat4{1.f}, translation);

    m_engine.renderEngine().vr().transform(transform);
}
