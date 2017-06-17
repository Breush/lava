#include <lava/magma/mesh.hpp>

#include <lava/chamber/logger.hpp>

#include "./glb-loader.hpp"
#include "./vulkan/mesh-impl.hpp"

using namespace lava;

Mesh::Mesh(RenderEngine& engine)
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
    logger.log().tab(-1) << "JSON Chunk:" << std::endl;
    logger.log().tab(1) << jsonChunk << std::endl;
    logger.log().tab(-1) << "BIN Chunk:" << std::endl;
    logger.log().tab(1) << binChunk << std::endl;
    logger.log().tab(-1);

    auto json = nlohmann::json::parse(jsonChunk.data);

    const auto& accessors = json["accessors"];
    const auto& bufferViews = json["bufferViews"];

    const auto& primitive = json["meshes"][0]["primitives"][0];
    const auto& attributes = primitive["attributes"];

    // Positions
    uint32_t positionsAccessorIndex = attributes["POSITION"];
    Accessor positionsAccessor(accessors[positionsAccessorIndex]);
    auto positions = access<glm::vec3>(positionsAccessor, bufferViews, binChunk.data);

    // Indices
    uint32_t indicesAccessorIndex = primitive["indices"];
    Accessor indicesAccessor(accessors[indicesAccessorIndex]);
    auto indicesVector = access<uint16_t>(indicesAccessor, bufferViews, binChunk.data);

    // Some tricks here
    for (auto& v : positions) {
        auto z = v.z;
        v.z = v.y;
        v.y = z;
        v *= 0.005;
    }

    logger.log() << "Vertices count: " << positions.size() << std::endl;
    logger.log() << "Indices count: " << indicesVector.size() << std::endl;

    verticesCount(positions.size());
    verticesPositions(positions);
    indices(indicesVector);
}

void Mesh::verticesCount(const uint32_t count)
{
    m_impl->verticesCount(count);
}

void Mesh::verticesPositions(const std::vector<glm::vec3>& positions)
{
    m_impl->verticesPositions(positions);
}

void Mesh::verticesColors(const std::vector<glm::vec3>& colors)
{
    m_impl->verticesColors(colors);
}

void Mesh::verticesUvs(const std::vector<glm::vec2>& uvs)
{
    m_impl->verticesUvs(uvs);
}

void Mesh::indices(const std::vector<uint16_t>& indices)
{
    m_impl->indices(indices);
}
