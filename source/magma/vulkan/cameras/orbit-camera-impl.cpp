#include "./orbit-camera-impl.hpp"

using namespace lava;

OrbitCamera::Impl::Impl(RenderEngine& engine)
    : m_engine(engine.impl())
{
}

OrbitCamera::Impl::~Impl()
{
}

void OrbitCamera::Impl::position(const glm::vec3& position)
{
    m_position = position;
}

void OrbitCamera::Impl::target(const glm::vec3& target)
{
    m_target = target;
}
