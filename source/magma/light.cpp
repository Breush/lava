#include <lava/magma/light.hpp>

#include "./aft-vulkan/light-aft.hpp"

using namespace lava::magma;

Light::Light(RenderScene& scene)
    : m_scene(scene)
{
    new (&aft()) LightAft(*this, m_scene.impl());
}

Light::~Light()
{
    aft().~LightAft();
}

// ----- Rendering

RenderImage Light::shadowsRenderImage() const
{
    return aft().foreShadowsRenderImage();
}

// ----- Controller-only

void Light::uboChanged()
{
    aft().foreUboChanged();
}
