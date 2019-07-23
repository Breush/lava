#include <lava/magma/scene.hpp>

#include <lava/magma/camera.hpp>
#include <lava/magma/light.hpp>
#include <lava/magma/material.hpp>
#include <lava/magma/mesh.hpp>
#include <lava/magma/texture.hpp>

#include "./aft-vulkan/scene-aft.hpp"
#include "./vulkan/holders/buffer-holder.hpp"
#include "./vulkan/stages/shadows-stage.hpp"

using namespace lava::magma;

Scene::Scene(RenderEngine& engine)
    : m_engine(engine)
{
    new (&aft()) SceneAft(*this, engine);

    // Fallback material
    m_fallbackMaterial = m_materialAllocator.allocate<Material>(*this, "fallback");
    add(*m_fallbackMaterial);
}

Scene::~Scene()
{
    // @note This cleanup is just to please the validation layers,
    // because eveything is going to be removed anyway,
    // and no more used.

    for (auto light : m_lights) {
        m_lightAllocator.deallocate(light);
    }

    for (auto camera : m_cameras) {
        m_cameraAllocator.deallocate(camera);
    }

    for (auto material : m_materials) {
        m_materialAllocator.deallocate(material);
    }

    for (auto texture : m_textures) {
        m_textureAllocator.deallocate(texture);
    }

    for (auto mesh : m_meshes) {
        m_meshAllocator.deallocate(mesh);
    }

    // @note Keep last, deleting the aft after all resources
    // have been freed.
    aft().~SceneAft();
}

// ----- Adders

void Scene::add(Light& light)
{
    m_lights.emplace_back(&light);
    aft().foreAdd(light);
}

void Scene::add(Camera& camera)
{
    m_cameras.emplace_back(&camera);
    aft().foreAdd(camera);
}

void Scene::add(Material& material)
{
    m_materials.emplace_back(&material);
    aft().foreAdd(material);
}

void Scene::add(Texture& texture)
{
    m_textures.emplace_back(&texture);
    aft().foreAdd(texture);
}

void Scene::add(Mesh& mesh)
{
    m_meshes.emplace_back(&mesh);
    aft().foreAdd(mesh);
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

void Scene::remove(const Material& material)
{
    aft().foreRemove(material);
}

void Scene::remove(const Texture& texture)
{
    aft().foreRemove(texture);
}

void Scene::remove(const Mesh& mesh)
{
    aft().foreRemove(mesh);
}

void Scene::removeUnsafe(const Light& light)
{
    for (auto iLight = m_lights.begin(); iLight != m_lights.end(); ++iLight) {
        if (*iLight == &light) {
            m_lightAllocator.deallocate(&light);
            m_lights.erase(iLight);
            break;
        }
    }
}

void Scene::removeUnsafe(const Camera& camera)
{
    for (auto iCamera = m_cameras.begin(); iCamera != m_cameras.end(); ++iCamera) {
        if (*iCamera == &camera) {
            m_cameraAllocator.deallocate(&camera);
            m_cameras.erase(iCamera);
            break;
        }
    }
}

void Scene::removeUnsafe(const Material& material)
{
    for (auto iMaterial = m_materials.begin(); iMaterial != m_materials.end(); ++iMaterial) {
        if (*iMaterial == &material) {
            m_materialAllocator.deallocate(&material);
            m_materials.erase(iMaterial);
            break;
        }
    }
}

void Scene::removeUnsafe(const Texture& texture)
{
    for (auto iTexture = m_textures.begin(); iTexture != m_textures.end(); ++iTexture) {
        if (*iTexture == &texture) {
            m_textureAllocator.deallocate(&texture);
            m_textures.erase(iTexture);
            break;
        }
    }
}

void Scene::removeUnsafe(const Mesh& mesh)
{
    for (auto iMesh = m_meshes.begin(); iMesh != m_meshes.end(); ++iMesh) {
        if (*iMesh == &mesh) {
            m_meshAllocator.deallocate(&mesh);
            m_meshes.erase(iMesh);
            break;
        }
    }
}

// ----- Environment

void Scene::environmentTexture(const Texture* texture)
{
    aft().foreEnvironmentTexture(texture);
}
