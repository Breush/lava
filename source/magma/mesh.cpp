#include <lava/magma/mesh.hpp>

#include <lava/chamber/logger.hpp>
#include <lava/chamber/pimpl.hpp>
#include <lava/magma/mrr-material.hpp>

#include <stb/stb_image.h>

#include "./glb-loader.hpp"
#include "./vulkan/mesh-impl.hpp"

using namespace lava;

$pimpl_class(Mesh, RenderEngine&, engine);

Mesh::Mesh(RenderEngine& engine, const std::string& fileName)
    : Mesh(engine)
{
    load(fileName);
}

$pimpl_method(Mesh, void, verticesCount, const uint32_t, count);
$pimpl_method(Mesh, void, verticesPositions, const std::vector<glm::vec3>&, positions);
$pimpl_method(Mesh, void, verticesColors, const std::vector<glm::vec3>&, colors);
$pimpl_method(Mesh, void, verticesUvs, const std::vector<glm::vec2>&, uvs);
$pimpl_method(Mesh, void, indices, const std::vector<uint16_t>&, indices);
$pimpl_method(Mesh, void, material, const MrrMaterial&, material);

void Mesh::load(const std::string& fileName)
{
    logger.info("magma.mesh") << "Loading file " << fileName << std::endl;
    logger.log().tab(1);

    std::ifstream file(fileName, std::ifstream::binary);

    Header header;
    Chunk jsonChunk;
    Chunk binChunk;
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
        logger.error("magma.mesh") << "Invalid mode " << primitive["mode"] << " for primitive."
                                   << " Only 4 (TRIANGLES) is supported." << std::endl;
        return;
    }

    // Positions
    uint32_t positionsAccessorIndex = attributes["POSITION"];
    Accessor positionsAccessor(accessors[positionsAccessorIndex]);
    auto positions = access<glm::vec3>(positionsAccessor, bufferViews, binChunk.data);

    // Fixing axes conventions and scaling
    for (auto& v : positions) {
        auto z = v.z;
        v.z = v.y;
        v.y = -z;
        v *= 0.007;
    }

    // UVs
    uint32_t uv1sAccessorIndex = attributes["TEXCOORD_0"];
    Accessor uv1sAccessor(accessors[uv1sAccessorIndex]);
    auto uv1s = access<glm::vec2>(uv1sAccessor, bufferViews, binChunk.data);

    // Indices
    uint32_t indicesAccessorIndex = primitive["indices"];
    Accessor indicesAccessor(accessors[indicesAccessorIndex]);
    auto indices = access<uint16_t>(indicesAccessor, bufferViews, binChunk.data);

    // Material
    uint32_t materialIndex = primitive["material"];
    PbrMetallicRoughnessMaterial material(materials[materialIndex]);
    MrrMaterial mrrMaterial;

    // Material textures
    uint32_t textureIndex = material.baseColorTextureIndex;
    Texture texture(textures[textureIndex]);
    Image image(images[texture.source]);

    int texWidth, texHeight, texChannels;
    auto imageData = access(bufferViews[image.bufferView], binChunk.data);
    auto pixels = stbi_load_from_memory(imageData.data(), imageData.size(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    std::vector<uint8_t> pixelsVector(texWidth * texHeight);
    memmove(pixelsVector.data(), pixels, pixelsVector.size());
    mrrMaterial.baseColor(pixelsVector, texWidth, texHeight, texChannels);
    stbi_image_free(pixels);

    std::cout << bufferViews[image.bufferView] << std::endl;

    logger.log() << "Vertices count: " << positions.size() << std::endl;
    logger.log() << "Indices count: " << indices.size() << std::endl;
    logger.log() << "Uv1s count: " << uv1s.size() << std::endl;

    m_impl->verticesCount(positions.size());
    m_impl->verticesPositions(positions);
    m_impl->verticesUvs(uv1s);
    m_impl->indices(indices);
    m_impl->material(mrrMaterial);
}
