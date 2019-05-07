#include <lava/sill/makers/glb-mesh.hpp>

#include <lava/sill/components/mesh-component.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/game-entity.hpp>
#include <lava/sill/material.hpp>
#include <lava/sill/mesh-node.hpp>
#include <lava/sill/mesh-primitive.hpp>
#include <lava/sill/texture.hpp>

#include "../glb/loader.hpp"

/**
 * @note It is very important to be able to write this
 * kind of file (generating meshes) without any access
 * to impl(). Otherwise, it means that the end user
 * won't be able to create its own meshes from scratch.
 */

using namespace lava;
using namespace lava::chamber;
using namespace lava::sill;

namespace {
    struct CacheData {
        std::unordered_map<uint32_t, Texture*> textures;
        std::unordered_map<uint32_t, Material*> materials;
        std::unordered_map<uint32_t, bool> materialTranslucencies;
    };

    using PixelsCallback = std::function<void(uint8_t*, uint32_t, uint32_t)>;

    void setTexture(GameEngine& engine, Material& material, const std::string& uniformName, uint32_t textureIndex,
                    const glb::Chunk& binChunk, const nlohmann::json& json, CacheData& cacheData,
                    PixelsCallback pixelsCallback = nullptr)
    {
        if (textureIndex == -1u) return;

        if (cacheData.textures.find(textureIndex) != cacheData.textures.end()) {
            auto& rmTexture = *cacheData.textures.at(textureIndex);
            material.set(uniformName, rmTexture);
        }
        else {
            glb::Texture texture(json["textures"][textureIndex]);
            glb::Image image(json["images"][texture.source]);

            int texWidth, texHeight;
            glb::BufferView imageBufferView(json["bufferViews"][image.bufferView]);
            auto imageVectorView = imageBufferView.get(binChunk.data);

            // @todo We might want to choose the number of channels one day...
            auto pixels = stbi_load_from_memory(imageVectorView.data(), imageVectorView.size(), &texWidth, &texHeight, nullptr,
                                                STBI_rgb_alpha);
            if (pixelsCallback) pixelsCallback(pixels, texWidth, texHeight);

            auto& rmTexture = engine.make<Texture>();
            rmTexture.loadFromMemory(pixels, texWidth, texHeight, 4u);
            material.set(uniformName, rmTexture);

            cacheData.textures[textureIndex] = &rmTexture;
            stbi_image_free(pixels);
        }
    }

    void setOrmTexture(GameEngine& engine, Material& material, uint32_t occlusionTextureIndex,
                       uint32_t metallicRoughnessTextureIndex, const glb::Chunk& binChunk, const nlohmann::json& json,
                       CacheData& cacheData)
    {
        // Both exist and are the same or both do not exist
        if (occlusionTextureIndex == metallicRoughnessTextureIndex) {
            setTexture(engine, material, "ormMap", metallicRoughnessTextureIndex, binChunk, json, cacheData);
            return;
        }

        // Both exist
        if (occlusionTextureIndex != -1u && metallicRoughnessTextureIndex != -1u) {
            // @todo Implement by merging the two. Be careful if the two sizes differ.
            logger.warning("sill.makers.glb-mesh")
                << "Occlusion and metallic-roughness do not match the same texture." << std::endl;
            logger.error("sill.makers.glb-mesh") << "This is currently not handled by the GLB importer." << std::endl;
            return;
        }

        // Occlusion missing, forcing it to 255u.
        if (occlusionTextureIndex == -1u) {
            setTexture(engine, material, "ormMap", metallicRoughnessTextureIndex, binChunk, json, cacheData,
                       [](uint8_t* pixels, uint32_t width, uint32_t height) {
                           auto length = 4u * width * height;
                           for (auto i = 0u; i < length; i += 4u) {
                               pixels[i] = 255u;
                           }
                       });
            return;
        }

        // Roughness-metallic missing
        if (metallicRoughnessTextureIndex == -1u) {
            logger.warning("sill.makers.glb-mesh") << "Metallic-roughness does not exist but occlusion does." << std::endl;
            logger.error("sill.makers.glb-mesh") << "This is currently not handled by the GLB importer." << std::endl;
            return;
        }
    }

    std::unique_ptr<Mesh> loadMesh(GameEntity& entity, uint32_t meshIndex, const glb::Chunk& binChunk, const nlohmann::json& json,
                                   CacheData& cacheData, bool flipTriangles)
    {
        PROFILE_FUNCTION();

        glb::Mesh mesh(json["meshes"][meshIndex]);

        auto& engine = entity.engine();
        const auto& accessors = json["accessors"];
        const auto& bufferViews = json["bufferViews"];
        const auto& materials = json.find("materials");

        auto meshData = std::make_unique<Mesh>();
        meshData->name(mesh.name);

        // Each primitive will consist in one magma::Mesh
        for (auto primitiveIndex = 0u; primitiveIndex < mesh.primitives.size(); ++primitiveIndex) {
            const auto& primitive = mesh.primitives[primitiveIndex];

            // Check mode TRIANGLES validity
            if (primitive.mode != 4u) {
                logger.error("sill.makers.glb-mesh") << "Invalid mode " << primitive.mode << " for primitive."
                                                     << " Only 4 (TRIANGLES) is supported." << std::endl;
            }

            // Positions
            auto positions =
                glb::Accessor(accessors[primitive.positionsAccessorIndex]).get<glm::vec3>(bufferViews, binChunk.data);

            // UVs
            VectorView<glm::vec2> uv1s;
            if (primitive.uv1sAccessorIndex != -1u) {
                uv1s = glb::Accessor(accessors[primitive.uv1sAccessorIndex]).get<glm::vec2>(bufferViews, binChunk.data);
            }

            // Normals
            VectorView<glm::vec3> normals;
            if (primitive.normalsAccessorIndex != -1u) {
                normals = glb::Accessor(accessors[primitive.normalsAccessorIndex]).get<glm::vec3>(bufferViews, binChunk.data);
            }
            else {
                // @todo Compute flat normals.
                logger.error("sill.makers.glb-mesh")
                    << "No normals found in mesh " << meshIndex << ", primitive " << primitiveIndex << "." << std::endl
                    << "Currently not generating flat normals." << std::endl;
            }

            // Tangents
            VectorView<glm::vec4> tangents;
            if (primitive.tangentsAccessorIndex != -1u) {
                tangents = glb::Accessor(accessors[primitive.tangentsAccessorIndex]).get<glm::vec4>(bufferViews, binChunk.data);
            }
            else {
                // @todo Otherwise, auto compute tangents nicely.
                logger.warning("sill.makers.glb-mesh")
                    << "No tangents found in mesh " << meshIndex << ", primitive " << primitiveIndex << "." << std::endl
                    << "Some materials might not render correctly." << std::endl;
            }

            // Material
            bool translucent = false;
            Material* rmMaterial = nullptr;
            if (cacheData.materials.find(primitive.materialIndex) != cacheData.materials.end()) {
                rmMaterial = cacheData.materials.at(primitive.materialIndex);
                translucent = cacheData.materialTranslucencies.at(primitive.materialIndex);
            }
            else if (materials != json.end()) {
                glb::PbrMetallicRoughnessMaterial material((*materials)[primitive.materialIndex]);
                rmMaterial = &engine.make<Material>("roughness-metallic");
                translucent = material.translucent;

                // Material textures
                rmMaterial->set("albedoColor", material.baseColorFactor);
                setTexture(engine, *rmMaterial, "albedoMap", material.baseColorTextureIndex, binChunk, json, cacheData);
                setTexture(engine, *rmMaterial, "normalMap", material.normalTextureIndex, binChunk, json, cacheData);
                setTexture(engine, *rmMaterial, "emissiveMap", material.emissiveTextureIndex, binChunk, json, cacheData);
                setOrmTexture(engine, *rmMaterial, material.occlusionTextureIndex, material.metallicRoughnessTextureIndex,
                              binChunk, json, cacheData);

                cacheData.materials[primitive.materialIndex] = rmMaterial;
                cacheData.materialTranslucencies[primitive.materialIndex] = translucent;
            }

            // Apply the geometry
            auto& meshPrimitive = meshData->addPrimitive(entity.engine());
            meshPrimitive.verticesCount(positions.size());
            meshPrimitive.verticesPositions(positions);
            meshPrimitive.verticesNormals(normals);
            meshPrimitive.verticesTangents(tangents);
            meshPrimitive.verticesUvs(uv1s);
            if (rmMaterial != nullptr) meshPrimitive.material(*rmMaterial);
            meshPrimitive.translucent(translucent);

            // Indices
            auto indicesComponentType = accessors[primitive.indicesAccessorIndex]["componentType"];
            if (indicesComponentType == 5122) {
                auto indices = glb::Accessor(accessors[primitive.indicesAccessorIndex]).get<uint16_t>(bufferViews, binChunk.data);
                meshPrimitive.indices(indices, flipTriangles);
            }
            else if (indicesComponentType == 5121) {
                auto indices = glb::Accessor(accessors[primitive.indicesAccessorIndex]).get<uint8_t>(bufferViews, binChunk.data);
                meshPrimitive.indices(indices, flipTriangles);
            }
            else {
                logger.warning("sill.makers.glb-mesh")
                    << "Indices component type " << indicesComponentType << " not handled." << std::endl;
            }
        }

        return std::move(meshData);
    }

    MeshNode* loadNode(GameEntity& entity, uint32_t nodeIndex, std::vector<MeshNode>& meshNodes, const glb::Chunk& binChunk,
                       const nlohmann::json& json, CacheData& cacheData)
    {
        glb::Node node(json["nodes"][nodeIndex]);

        if (node.children.empty() && node.meshIndex == -1u) {
            logger.warning("sill.makers.glb-maker")
                << "Node '" << node.name << "' is empty and has no children, so it has been removed." << std::endl;
            return nullptr;
        }

        // @note We have one MeshNode per glb::Node,
        // and one for each primitive within its mesh (if any).
        meshNodes.emplace_back();
        auto& meshNode = meshNodes.back();

        meshNode.name = node.name;
        meshNode.transform = node.transform;

        // @note From https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#transformations
        // > If the determinant of the transform is a negative value,
        // > the winding order of the mesh triangle faces should be reversed.
        // > This supports negative scales for mirroring geometry.
        bool flipTriangles = (glm::determinant(meshNode.transform) < 0);

        // Load geometry if any
        if (node.meshIndex != -1u) {
            meshNode.mesh = loadMesh(entity, node.meshIndex, binChunk, json, cacheData, flipTriangles);
        }

        // Recurse over children
        for (auto child : node.children) {
            auto childNode = loadNode(entity, child, meshNodes, binChunk, json, cacheData);
            if (childNode != nullptr) {
                meshNode.children.emplace_back(childNode);
            }
        }

        return &meshNode;
    }
}

std::function<void(MeshComponent&)> makers::glbMeshMaker(const std::string& fileName)
{
    logger.info("sill.makers.glb-mesh") << "Loading file " << fileName << std::endl;

    std::ifstream file(fileName, std::ifstream::binary);

    if (!file.is_open()) {
        logger.error("sill.makers.glb-mesh") << "Unable to read file " << fileName << std::endl;
    }

    // ----- Header

    glb::Header header;
    glb::Chunk jsonChunk;
    glb::Chunk binChunk;
    file >> header >> jsonChunk >> binChunk;

    auto json = nlohmann::json::parse(jsonChunk.data);

    return [=](MeshComponent& meshComponent) {
        PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

        // ----- Resources pre-allocation

        // Caching
        CacheData cacheData;

        // The mesh nodes array
        std::vector<MeshNode> meshNodes;
        meshNodes.reserve(json["nodes"].size() + 1u);

        // ----- Scene loading

        uint32_t rootScene = json["scene"];
        auto nodes = json["scenes"][rootScene]["nodes"];

        meshNodes.emplace_back();
        auto& rootNode = meshNodes.back();
        rootNode.name = fileName;

        for (uint32_t nodeIndex : nodes) {
            auto node = loadNode(meshComponent.entity(), nodeIndex, meshNodes, binChunk, json, cacheData);
            if (node != nullptr) {
                rootNode.children.emplace_back(node);
            }
        }

        // ----- Fixing convention of glTF (right-handed, front is +X, left is +Z)

        rootNode.transform = rotationMatrixFromAxes(Axis::PositiveZ, Axis::PositiveX, Axis::PositiveY);

        meshComponent.nodes(std::move(meshNodes));

        logger.info("sill.makers.glb-mesh") << "Generated mesh component for " << fileName << std::endl;
    };
}
