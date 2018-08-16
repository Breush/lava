#include "./camera-component-impl.hpp"

#include "../game-engine-impl.hpp"

using namespace lava::sill;

CameraComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
{
    // @fixme As with physics engine or font manager, we shouldn't need
    // to access impl() of engine here...
    auto& engine = m_entity.engine().impl();

    // @todo We might want to have a way to specify that this camera should be
    // using the window extent but user specific one. For instance, if the result
    // should be used as a mirror.
    m_camera = &engine.renderScene().make<magma::OrbitCamera>(engine.windowRenderTarget().extent());

    // @todo Let the viewport be configurable too...
    engine.renderEngine().addView(*m_camera, engine.windowRenderTarget(), Viewport{0, 0, 1, 1});

    // @fixme Have a way to remove this callback when the component is destroyed.
    engine.onWindowExtentChanged([this](Extent2d extent) {
        m_updateDelay = 0.1f;
        m_extent = extent;
    });
}

CameraComponent::Impl::~Impl()
{
    // @todo In magma, we can't remove a camera so nicely for now...
}

void CameraComponent::Impl::update(float dt)
{
    // @note We delay updates as it might take a while
    // recontructing buffers and such.
    if (m_updateDelay > 0.f) {
        m_updateDelay -= dt;
        if (m_updateDelay <= 0.f) {
            m_camera->extent(m_extent);
        }
    }
}
