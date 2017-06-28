#include <lava/magma/meshes/mesh.hpp>

#include <lava/chamber/logger.hpp>
#include <lava/chamber/pimpl.hpp>
#include <lava/magma/materials/mrr-material.hpp>

#include <stb/stb_image.h>

#include "../glb/loader.hpp"
#include "../vulkan/meshes/mesh-impl.hpp"

using namespace lava;

Mesh::Mesh(RenderEngine& engine)
    : m_engine(engine)
{
    m_impl = new Impl(engine);
}

Mesh::Mesh(RenderEngine& engine, const std::string& fileName)
    : Mesh(engine)
{
    load(fileName);
}

Mesh::~Mesh()
{
    delete m_impl;
}

$pimpl_method(Mesh, IMesh::UserData, render, IMesh::UserData, data);

$pimpl_method(Mesh, void, verticesCount, const uint32_t, count);
$pimpl_method(Mesh, void, verticesPositions, const std::vector<glm::vec3>&, positions);
$pimpl_method(Mesh, void, verticesNormals, const std::vector<glm::vec3>&, normals);
$pimpl_method(Mesh, void, verticesColors, const std::vector<glm::vec3>&, colors);
$pimpl_method(Mesh, void, verticesUvs, const std::vector<glm::vec2>&, uvs);
$pimpl_method(Mesh, void, indices, const std::vector<uint16_t>&, indices);
$pimpl_method(Mesh, void, material, const MrrMaterial&, material);

void Mesh::load(const std::string& fileName)
{
    logger.info("magma.mesh.glb-loader") << "Loading file " << fileName << std::endl;
    logger.log().tab(1);

    std::ifstream file(fileName, std::ifstream::binary);

    glb::Header header;
    glb::Chunk jsonChunk;
    glb::Chunk binChunk;
    file >> header >> jsonChunk >> binChunk;

    logger.log() << "Header:" << std::endl;
    logger.log().tab(1) << header << std::endl;
    logger.log().tab(-1) << "JSON chunk:" << std::endl;
    logger.log().tab(1) << jsonChunk << std::endl;
    logger.log().tab(-1) << "BIN chunk:" << std::endl;
    logger.log().tab(1) << binChunk << std::endl;
    logger.log().tab(-1);

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
        logger.error("magma.mesh.glb-loader") << "Invalid mode " << primitive["mode"] << " for primitive."
                                              << " Only 4 (TRIANGLES) is supported." << std::endl;
        return;
    }

    // Positions
    if (attributes.find("POSITION") == attributes.end()) {
        logger.error("magma.mesh.glb-loader") << "No position found." << std::endl;
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
    auto& mrrMaterial = m_engine.make<MrrMaterial>();

    // Material textures
    if (material.baseColorTextureIndex != -1u) {
        logger.log() << "Base color texture found." << std::endl;

        uint32_t textureIndex = material.baseColorTextureIndex;
        glb::Texture texture(textures[textureIndex]);
        glb::Image image(images[texture.source]);

        int texWidth, texHeight;
        glb::BufferView imageBufferView(bufferViews[image.bufferView]);
        auto imageData = imageBufferView.get(binChunk.data);
        auto pixels = stbi_load_from_memory(imageData.data(), imageData.size(), &texWidth, &texHeight, nullptr, STBI_rgb_alpha);
        std::vector<uint8_t> pixelsVector(texWidth * texHeight * 4);
        memmove(pixelsVector.data(), pixels, pixelsVector.size());
        mrrMaterial.baseColor(pixelsVector, texWidth, texHeight, 4);
        stbi_image_free(pixels);
    }
    if (material.normalTextureIndex != -1u) {
        logger.log() << "Normal texture found." << std::endl;

        uint32_t textureIndex = material.normalTextureIndex;
        glb::Texture texture(textures[textureIndex]);
        glb::Image image(images[texture.source]);

        int texWidth, texHeight;
        glb::BufferView imageBufferView(bufferViews[image.bufferView]);
        auto imageData = imageBufferView.get(binChunk.data);
        auto pixels = stbi_load_from_memory(imageData.data(), imageData.size(), &texWidth, &texHeight, nullptr, STBI_rgb_alpha);
        std::vector<uint8_t> pixelsVector(texWidth * texHeight * 4);
        memmove(pixelsVector.data(), pixels, pixelsVector.size());
        mrrMaterial.normal(pixelsVector, texWidth, texHeight, 4);
        stbi_image_free(pixels);
    }
    if (material.metallicRoughnessTextureIndex != -1u) {
        logger.log() << "Metallic roughness texture found." << std::endl;

        uint32_t textureIndex = material.metallicRoughnessTextureIndex;
        glb::Texture texture(textures[textureIndex]);
        glb::Image image(images[texture.source]);

        int texWidth, texHeight;
        glb::BufferView imageBufferView(bufferViews[image.bufferView]);
        auto imageData = imageBufferView.get(binChunk.data);
        auto pixels = stbi_load_from_memory(imageData.data(), imageData.size(), &texWidth, &texHeight, nullptr, STBI_rgb_alpha);
        std::vector<uint8_t> pixelsVector(texWidth * texHeight * 4);
        memmove(pixelsVector.data(), pixels, pixelsVector.size());
        mrrMaterial.metallicRoughnessColor(pixelsVector, texWidth, texHeight, 4);
        stbi_image_free(pixels);
    }

    // All right, we're done!
    logger.log() << "Vertices count: " << positions.size() << std::endl;
    logger.log() << "Normals count: " << normals.size() << std::endl;
    logger.log() << "Indices count: " << indices.size() << std::endl;
    logger.log() << "Uv1s count: " << uv1s.size() << std::endl;

    m_impl->verticesCount(positions.size());
    m_impl->verticesPositions(positions);
    m_impl->verticesNormals(normals);
    m_impl->verticesTangents(tangents);
    m_impl->verticesUvs(uv1s);
    m_impl->indices(indices);
    m_impl->material(mrrMaterial);
}
