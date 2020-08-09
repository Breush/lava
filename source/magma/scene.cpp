#include <lava/magma/scene.hpp>

#include <lava/magma/camera.hpp>
#include <lava/magma/flat.hpp>
#include <lava/magma/light.hpp>
#include <lava/magma/material.hpp>
#include <lava/magma/mesh.hpp>
#include <lava/magma/texture.hpp>

#include "./aft-vulkan/camera-aft.hpp"
#include "./aft-vulkan/flat-aft.hpp"
#include "./aft-vulkan/light-aft.hpp"
#include "./aft-vulkan/material-aft.hpp"
#include "./aft-vulkan/mesh-aft.hpp"
#include "./aft-vulkan/scene-aft.hpp"
#include "./aft-vulkan/texture-aft.hpp"
#include "./vulkan/holders/buffer-holder.hpp"
#include "./vulkan/stages/shadows-stage.hpp"

using namespace lava::chamber;
using namespace lava::magma;

Scene::Scene(RenderEngine& engine)
    : m_engine(engine)
{
    new (&aft()) SceneAft(*this, engine);

    // Fallback material
    m_fallbackMaterial = makeMaterial("fallback");
}

Scene::~Scene()
{
    // @note This cleanup is just to please the validation layers,
    // because eveything is going to be removed anyway,
    // and no more used.

    m_fallbackMaterial = nullptr;

    for (auto light : m_lights) {
        m_lightAllocator.deallocate(light);
    }

    for (auto camera : m_cameras) {
        m_cameraAllocator.deallocate(camera);
    }

    for (auto mesh : m_meshes) {
        m_meshAllocator.deallocate(mesh);
    }

    for (auto flat : m_flats) {
        m_flatAllocator.deallocate(flat);
    }

    if (!m_materials.empty()) {
        logger.warning("magma.scene") << "Alive MaterialPtr: " << std::endl;
        for (const auto& material : m_materials) {
            logger.warning("magma.scene") << material << " " << material->hrid() << " (" << material->name() << ")" << std::endl;
        }
        logger.error("magma.scene") << "There are still MaterialPtr references alive. Please reset them all before destroying a scene." << std::endl;
    }

    // @note Keep last, deleting the aft after all resources
    // have been freed.
    aft().~SceneAft();
}

// ----- Rendering

void Scene::msaa(Msaa msaa)
{
    if (m_msaa == msaa) return;
    m_msaa = msaa;

    aft().foreMsaaChanged();
}

// ----- Makers

// :RuntimeAft @note Any resource is in fact allocated with more space,
// thanks to the bucket allocator allocateSized() function.
// Doing so allows us to have good memory layout, keeping the aft near
// the resource data.

Light& Scene::makeLight()
{
    constexpr const auto size = sizeof(std::aligned_union<0, Light>::type) + sizeof(LightAft);
    auto resource = m_lightAllocator.allocateSized<Light>(size, *this);

    m_lights.emplace_back(resource);
    aft().foreAdd(*resource);
    return *resource;
}

Camera& Scene::makeCamera(Extent2d extent)
{
    constexpr const auto size = sizeof(std::aligned_union<0, Camera>::type) + sizeof(CameraAft);
    auto resource = m_cameraAllocator.allocateSized<Camera>(size, *this, extent);

    m_cameras.emplace_back(resource);
    aft().foreAdd(*resource);
    return *resource;
}

MaterialPtr Scene::makeMaterial(const std::string& hrid)
{
    constexpr const auto size = sizeof(std::aligned_union<0, Material>::type) + sizeof(MaterialAft);
    auto resource = m_materialAllocator.allocateSized<Material>(size, *this, hrid);
    MaterialPtr resourcePtr(resource, [this](Material* material) {
        forget(*material);
    });

    m_materials.emplace_back(resource);
    aft().foreAdd(*resource);
    return resourcePtr;
}

Mesh& Scene::makeMesh()
{
    constexpr const auto size = sizeof(std::aligned_union<0, Mesh>::type) + sizeof(MeshAft);
    auto resource = m_meshAllocator.allocateSized<Mesh>(size, *this);

    m_meshes.emplace_back(resource);
    aft().foreAdd(*resource);
    return *resource;
}

Flat& Scene::makeFlat()
{
    constexpr const auto size = sizeof(std::aligned_union<0, Flat>::type) + sizeof(FlatAft);
    auto resource = m_flatAllocator.allocateSized<Flat>(size, *this);

    m_flats.emplace_back(resource);
    aft().foreAdd(*resource);
    return *resource;
}

// ----- Removers

void Scene::remove(const Light& light)
{
    aft().foreRemove(light);
}

void Scene::remove(const Camera& camera)
{
    aft().foreRemove(camera);
}

void Scene::remove(const Mesh& mesh)
{
    aft().foreRemove(mesh);
}

void Scene::remove(const Flat& flat)
{
    aft().foreRemove(flat);
}

void Scene::forget(Material& material)
{
    for (auto iMaterial = m_materials.begin(); iMaterial != m_materials.end(); ++iMaterial) {
        if (*iMaterial == &material) {
            m_materialAllocator.deallocate(&material);
            m_materials.erase(iMaterial);
            break;
        }
    }
}

void Scene::removeUnsafe(const Light& light)
{
    for (auto iLight = m_lights.begin(); iLight != m_lights.end(); ++iLight) {
        if (*iLight == &light) {
            m_lightAllocator.deallocate(*iLight);
            m_lights.erase(iLight);
            break;
        }
    }
}

void Scene::removeUnsafe(const Camera& camera)
{
    for (auto iCamera = m_cameras.begin(); iCamera != m_cameras.end(); ++iCamera) {
        if (*iCamera == &camera) {
            m_cameraAllocator.deallocate(*iCamera);
            m_cameras.erase(iCamera);
            break;
        }
    }
}

void Scene::removeUnsafe(const Mesh& mesh)
{
    for (auto iMesh = m_meshes.begin(); iMesh != m_meshes.end(); ++iMesh) {
        if (*iMesh == &mesh) {
            m_meshAllocator.deallocate(*iMesh);
            m_meshes.erase(iMesh);
            break;
        }
    }
}

void Scene::removeUnsafe(const Flat& flat)
{
    for (auto iFlat = m_flats.begin(); iFlat != m_flats.end(); ++iFlat) {
        if (*iFlat == &flat) {
            m_flatAllocator.deallocate(*iFlat);
            m_flats.erase(iFlat);
            break;
        }
    }
}

// ----- Environment

void Scene::environmentTexture(const TexturePtr& texture)
{
    aft().foreEnvironmentTexture(texture);
}
