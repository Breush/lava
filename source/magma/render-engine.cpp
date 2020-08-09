#include <lava/magma/render-engine.hpp>

#include <lava/magma/camera.hpp>
#include <lava/magma/texture.hpp>

#include "./aft-vulkan/scene-aft.hpp"
#include "./aft-vulkan/texture-aft.hpp"
#include "./vulkan/render-engine-impl.hpp"

using namespace lava;
using namespace lava::chamber;
using namespace lava::magma;

RenderEngine::RenderEngine()
{
    m_impl = new RenderEngine::Impl(*this);

    // Register fallback material
    // @todo Should be inlined as const string somehow
    registerMaterialFromFile("fallback", "./data/shaders/materials/fallback-material.shmag");
}

RenderEngine::~RenderEngine()
{
    delete m_impl;

    if (!m_textures.empty()) {
        logger.warning("magma.render-engine") << "Alive TexturePtr: " << std::endl;
        for (const auto& texture : m_textures) {
            logger.warning("magma.render-engine") << texture.first << " (" << texture.first->name() << ")" << std::endl;
        }
        logger.error("magma.render-engine") << "There are still TexturePtr references alive. Please reset them all before destroying the render engine." << std::endl;
    }
}

$pimpl_method(RenderEngine, void, update);
$pimpl_method(RenderEngine, void, draw);
$pimpl_method_const(RenderEngine, const MaterialInfo&, materialInfo, const std::string&, hrid);
$pimpl_method_const(RenderEngine, const MaterialInfo*, materialInfoIfExists, const std::string&, hrid);
$pimpl_method(RenderEngine, uint32_t, registerMaterialFromFile, const std::string&, hrid, const fs::Path&, shaderPath);

uint32_t RenderEngine::addView(Camera& camera, IRenderTarget& renderTarget, const Viewport& viewport)
{
    return addView(camera.renderImage(), renderTarget, viewport);
}

$pimpl_method(RenderEngine, uint32_t, addView, RenderImage, renderImage, IRenderTarget&, renderTarget, const Viewport&, viewport);
$pimpl_method(RenderEngine, void, removeView, uint32_t, viewId);

//----- Makers

// :RuntimeAft

Scene& RenderEngine::makeScene()
{
    constexpr const auto size = sizeof(std::aligned_union<0, Scene>::type) + sizeof(SceneAft);
    auto resource = m_sceneAllocator.allocateSized<Scene>(size, *this);
    add(*resource);
    return *resource;
}

TexturePtr RenderEngine::makeTexture(const std::string& imagePath)
{
    constexpr const auto size = sizeof(std::aligned_union<0, Texture>::type) + sizeof(TextureAft);
    auto resource = m_textureAllocator.allocateSized<Texture>(size, *this, imagePath);
    TexturePtr resourcePtr(resource, [this](Texture* texture) {
        forget(*texture);
    });

    m_textures.emplace(resource, resourcePtr);
    return resourcePtr;
}

TexturePtr RenderEngine::findTexture(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    auto hash = Texture::hash(pixels, width, height, channels);

    // @todo We're computing the hash twice for the texture that do not match,
    // because we add it afterwards, there might be a way to return the hash info too.
    for (const auto& texture : m_textures) {
        auto texturePtr = texture.second.lock();
        if (texturePtr->cube() == false && texturePtr->hash() == hash) {
            return texturePtr;
        }
    }

    return nullptr;
}

//----- Adders

$pimpl_method(RenderEngine, void, add, Scene&, scene);

void RenderEngine::add(std::unique_ptr<IRenderTarget>&& renderTarget)
{
    m_impl->add(std::move(renderTarget));
}

//----- Removers

void RenderEngine::forget(Texture& texture)
{
    auto iTexture = m_textures.find(&texture);
    if (iTexture != m_textures.end()) {
        m_textureAllocator.deallocate(&texture);
        m_textures.erase(iTexture);
    }
}

//----- Extra

$pimpl_method(RenderEngine, void, logTrackingOnce);
