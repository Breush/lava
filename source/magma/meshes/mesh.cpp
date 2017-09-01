#include <lava/magma/meshes/mesh.hpp>

#include <lava/chamber/logger.hpp>
#include <lava/chamber/macros.hpp>
#include <lava/magma/materials/rm-material.hpp>

#include <stb/stb_image.h>

#include "../glb/loader.hpp"
#include "../vulkan/meshes/mesh-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

Mesh::Mesh(RenderScene& scene)
    : m_scene(scene)
{
    m_impl = new Impl(scene);
}

Mesh::Mesh(RenderScene& scene, const std::string& fileName)
    : Mesh(scene)
{
    load(fileName);
}

Mesh::~Mesh()
{
    delete m_impl;
}

// IMesh
IMesh::Impl& Mesh::interfaceImpl()
{
    return *m_impl;
}

$pimpl_method_const(Mesh, const glm::mat4&, worldTransform);
$pimpl_method(Mesh, void, positionAdd, const glm::vec3&, delta);

$pimpl_method(Mesh, void, verticesCount, const uint32_t, count);
$pimpl_method(Mesh, void, verticesPositions, const std::vector<glm::vec3>&, positions);
$pimpl_method(Mesh, void, verticesUvs, const std::vector<glm::vec2>&, uvs);
$pimpl_method(Mesh, void, verticesNormals, const std::vector<glm::vec3>&, normals);
$pimpl_method(Mesh, void, verticesTangents, const std::vector<glm::vec4>&, tangents);
$pimpl_method(Mesh, void, indices, const std::vector<uint16_t>&, indices);

$pimpl_method(Mesh, IMaterial&, material);
$pimpl_method(Mesh, void, material, IMaterial&, material);

void Mesh::load(const std::string& fileName)
{
    logger.info("magma.mesh") << "Loading file " << fileName << std::endl;

    std::ifstream file(fileName, std::ifstream::binary);

    if (!file.is_open()) {
        logger.error("magma.mesh") << "Unable to read file " << fileName << std::endl;
    }

    glb::Header header;
    glb::Chunk jsonChunk;
    glb::Chunk binChunk;
    file >> header >> jsonChunk >> binChunk;

    auto json = nlohmann::json::parse(jsonChunk.data);

    const auto& images = json["images"];
    const auto& textures = json["textures"];
    const auto& materials = json["materials"];
    const auto& accessors = json["accessors"];
    const auto& bufferViews = json["bufferViews"];

    const auto& primitive = json["meshes"][0]["primitives"][0];
    const auto& attributes = primitive["attributes"];

    // Check mode TRIANGLES validity
    if (primitive.find("mode") != primitive.end() && primitive["mode"] != 4u) {
        logger.error("magma.mesh") << "Invalid mode " << primitive["mode"] << " for primitive."
                                   << " Only 4 (TRIANGLES) is supported." << std::endl;
        return;
    }

    // Positions
    if (attributes.find("POSITION") == attributes.end()) {
        logger.error("magma.mesh") << "No position found." << std::endl;
    }

    uint32_t positionsAccessorIndex = attributes["POSITION"];
    glb::Accessor positionsAccessor(accessors[positionsAccessorIndex]);
    auto positions = positionsAccessor.get<glm::vec3>(bufferViews, binChunk.data);

    // Fixing axes conventions and scaling
    for (auto& v : positions) {
        auto z = v.z;
        v.z = v.y;
        v.y = -z;
        v *= 20;
    }

    // Normals
    uint32_t normalsAccessorIndex = attributes["NORMAL"];
    glb::Accessor normalsAccessor(accessors[normalsAccessorIndex]);
    auto normals = normalsAccessor.get<glm::vec3>(bufferViews, binChunk.data);

    // Tangents
    uint32_t tangentsAccessorIndex = attributes["TANGENT"];
    glb::Accessor tangentsAccessor(accessors[tangentsAccessorIndex]);
    auto tangents = tangentsAccessor.get<glm::vec4>(bufferViews, binChunk.data);

    // Fixing axes conventions
    for (auto& v : normals) {
        auto z = v.z;
        v.z = v.y;
        v.y = -z;
    }

    // UVs
    uint32_t uv1sAccessorIndex = attributes["TEXCOORD_0"];
    glb::Accessor uv1sAccessor(accessors[uv1sAccessorIndex]);
    auto uv1s = uv1sAccessor.get<glm::vec2>(bufferViews, binChunk.data);

    // Indices
    uint32_t indicesAccessorIndex = primitive["indices"];
    glb::Accessor indicesAccessor(accessors[indicesAccessorIndex]);
    auto indices = indicesAccessor.get<uint16_t>(bufferViews, binChunk.data);

    // Material
    uint32_t materialIndex = primitive["material"];
    glb::PbrMetallicRoughnessMaterial material(materials[materialIndex]);
    auto& rmMaterial = m_scene.make<RmMaterial>();

    // Material textures
    if (material.baseColorTextureIndex != -1u) {
        uint32_t textureIndex = material.baseColorTextureIndex;
        glb::Texture texture(textures[textureIndex]);
        glb::Image image(images[texture.source]);

        int texWidth, texHeight;
        glb::BufferView imageBufferView(bufferViews[image.bufferView]);
        auto imageData = imageBufferView.get(binChunk.data);
        auto pixels = stbi_load_from_memory(imageData.data(), imageData.size(), &texWidth, &texHeight, nullptr, STBI_rgb_alpha);
        std::vector<uint8_t> pixelsVector(texWidth * texHeight * 4);
        memmove(pixelsVector.data(), pixels, pixelsVector.size());
        rmMaterial.baseColor(pixelsVector, texWidth, texHeight, 4);
        stbi_image_free(pixels);
    }
    if (material.normalTextureIndex != -1u) {
        uint32_t textureIndex = material.normalTextureIndex;
        glb::Texture texture(textures[textureIndex]);
        glb::Image image(images[texture.source]);

        int texWidth, texHeight;
        glb::BufferView imageBufferView(bufferViews[image.bufferView]);
        auto imageData = imageBufferView.get(binChunk.data);
        auto pixels = stbi_load_from_memory(imageData.data(), imageData.size(), &texWidth, &texHeight, nullptr, STBI_rgb_alpha);
        std::vector<uint8_t> pixelsVector(texWidth * texHeight * 4);
        memmove(pixelsVector.data(), pixels, pixelsVector.size());
        rmMaterial.normal(pixelsVector, texWidth, texHeight, 4);
        stbi_image_free(pixels);
    }
    if (material.metallicRoughnessTextureIndex != -1u) {
        uint32_t textureIndex = material.metallicRoughnessTextureIndex;
        glb::Texture texture(textures[textureIndex]);
        glb::Image image(images[texture.source]);

        int texWidth, texHeight;
        glb::BufferView imageBufferView(bufferViews[image.bufferView]);
        auto imageData = imageBufferView.get(binChunk.data);
        auto pixels = stbi_load_from_memory(imageData.data(), imageData.size(), &texWidth, &texHeight, nullptr, STBI_rgb_alpha);
        std::vector<uint8_t> pixelsVector(texWidth * texHeight * 4);
        memmove(pixelsVector.data(), pixels, pixelsVector.size());
        rmMaterial.metallicRoughnessColor(pixelsVector, texWidth, texHeight, 4);
        stbi_image_free(pixels);
    }

    m_impl->verticesCount(positions.size());
    m_impl->verticesPositions(positions);
    m_impl->verticesNormals(normals);
    m_impl->verticesTangents(tangents);
    m_impl->verticesUvs(uv1s);
    m_impl->indices(indices);
    m_impl->material(rmMaterial);
}
