#include <lava/sill/makers/glb-mesh.hpp>

#include <lava/sill/components/mesh-component.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/game-entity.hpp>
#include <lava/sill/mesh-node.hpp>

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
        std::unique_ptr<ThreadPool> threadPool;
        std::unordered_map<uint32_t, magma::TexturePtr> textures;
        std::unordered_map<uint32_t, magma::MaterialPtr> materials;
        std::unordered_map<uint32_t, bool> materialTranslucencies;
        std::unordered_map<uint32_t, uint32_t> nodeIndices;

        // Key is textureId, value is a list of materials and uniformName waiting for that texture to be done.
        std::unordered_map<uint32_t, std::vector<std::pair<magma::Material*, std::string>>> pendingUniformBindings;
    };

    using PixelsCallback = std::function<void(uint8_t*, uint32_t, uint32_t)>;

    void setTexture(GameEngine& engine, magma::Material& material, const std::string& uniformName, uint32_t textureIndex,
                    const glb::Chunk& binChunk, const nlohmann::json& json, CacheData& cacheData,
                    const PixelsCallback& pixelsCallback = nullptr)
    {
        if (textureIndex == -1u) return;

        cacheData.pendingUniformBindings[textureIndex].emplace_back(&material, uniformName);

        // No need to reload already existing textures.
        if (cacheData.textures.find(textureIndex) != cacheData.textures.end()) {
            return;
        }

        glb::Texture texture(json["textures"][textureIndex]);
        glb::Image image(json["images"][texture.source]);

        glb::BufferView imageBufferView(json["bufferViews"][image.bufferView]);
        auto imageVectorView = imageBufferView.get(binChunk.data);

        // Will be set later in an other thread.
        cacheData.textures[textureIndex] = nullptr;

        if (!cacheData.threadPool) {
            cacheData.threadPool = std::make_unique<ThreadPool>();
        }

        cacheData.threadPool->job([&engine, &cacheData, textureIndex, imageVectorView, pixelsCallback] {
            // @todo We might want to choose the number of channels one day...
            int texWidth, texHeight;
            auto pixels = stbi_load_from_memory(imageVectorView.data(), imageVectorView.size(), &texWidth, &texHeight, nullptr,
                                                STBI_rgb_alpha);
            if (pixelsCallback) pixelsCallback(pixels, texWidth, texHeight);

            // @note Because this is threaded, a badly-made GLB can ask to load the same texture multiple times
            // if the data is duplicated. We cannot do anything at this level...
            // This findTexture is for already loaded textures from other meshes.
            auto existingTexture = engine.renderEngine().findTexture(pixels, texWidth, texHeight, 4u);
            if (existingTexture != nullptr) {
                cacheData.textures[textureIndex] = std::move(existingTexture);
            }
            else {
                auto texture = engine.renderEngine().makeTexture();
                texture->loadFromMemory(pixels, texWidth, texHeight, 4u);
                cacheData.textures[textureIndex] = std::move(texture);
            }

            stbi_image_free(pixels);
        });
    }

    void setOrmTexture(GameEngine& engine, magma::Material& material, uint32_t occlusionTextureIndex,
                       uint32_t metallicRoughnessTextureIndex, const glb::Chunk& binChunk, const nlohmann::json& json,
                       CacheData& cacheData)
    {
        // Occlusion missing, forcing it to 255u.
        if (occlusionTextureIndex == -1u && metallicRoughnessTextureIndex != -1u) {
            setTexture(engine, material, "roughnessMetallicMap", metallicRoughnessTextureIndex, binChunk, json, cacheData,
                       [](uint8_t* pixels, uint32_t width, uint32_t height) {
                           auto length = 4u * width * height;
                           for (auto i = 0u; i < length; i += 4u) {
                               pixels[i] = 255u;
                           }
                       });
            return;
        }

        // Every other case is valid.
        setTexture(engine, material, "occlusionMap", occlusionTextureIndex, binChunk, json, cacheData);
        setTexture(engine, material, "roughnessMetallicMap", metallicRoughnessTextureIndex, binChunk, json, cacheData);
    }

    std::unique_ptr<MeshGroup> loadMesh(MeshComponent& meshComponent, uint32_t meshIndex, const glb::Chunk& binChunk, const nlohmann::json& json,
                                        CacheData& cacheData, bool flipTriangles)
    {
        PROFILE_FUNCTION();

        glb::Mesh mesh(json["meshes"][meshIndex]);

        auto& engine = meshComponent.entity().engine();
        const auto& accessors = json["accessors"];
        const auto& bufferViews = json["bufferViews"];
        const auto& materials = json.find("materials");

        auto meshGroup = std::make_unique<MeshGroup>(meshComponent.scene());
        meshGroup->name(mesh.name);

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

            // Tangents
            VectorView<glm::vec4> tangents;
            if (primitive.tangentsAccessorIndex != -1u) {
                tangents = glb::Accessor(accessors[primitive.tangentsAccessorIndex]).get<glm::vec4>(bufferViews, binChunk.data);
            }

            // Material
            bool translucent = false;
            magma::MaterialPtr rmMaterial = nullptr;
            if (cacheData.materials.find(primitive.materialIndex) != cacheData.materials.end()) {
                rmMaterial = cacheData.materials.at(primitive.materialIndex);
                translucent = cacheData.materialTranslucencies.at(primitive.materialIndex);
            }
            else if (materials != json.end() && primitive.materialIndex != -1u) {
                // @note We need "roughness-metallic" material.
                if (engine.renderEngine().materialInfoIfExists("roughness-metallic") == nullptr) {
                    engine.registerMaterialFromFile("roughness-metallic", "./data/shaders/materials/rm-material.shmag");
                }

                glb::PbrMetallicRoughnessMaterial material((*materials)[primitive.materialIndex]);
                rmMaterial = engine.scene().makeMaterial("roughness-metallic");
                rmMaterial->name(material.name);
                translucent = material.translucent;

                // Material textures
                rmMaterial->set("albedoColor", material.baseColorFactor);
                rmMaterial->set("metallicFactor", material.metallicFactor);
                rmMaterial->set("roughnessFactor", material.roughnessFactor);
                setTexture(engine, *rmMaterial, "albedoMap", material.baseColorTextureIndex, binChunk, json, cacheData);
                setTexture(engine, *rmMaterial, "normalMap", material.normalTextureIndex, binChunk, json, cacheData);
                setTexture(engine, *rmMaterial, "emissiveMap", material.emissiveTextureIndex, binChunk, json, cacheData);
                setOrmTexture(engine, *rmMaterial, material.occlusionTextureIndex, material.metallicRoughnessTextureIndex,
                              binChunk, json, cacheData);

                cacheData.materials[primitive.materialIndex] = rmMaterial;
                cacheData.materialTranslucencies[primitive.materialIndex] = translucent;
            }

            // Apply the geometry
            auto& meshPrimitive = meshGroup->addPrimitive();
            meshPrimitive.verticesCount(positions.size());

            auto indicesComponentType = accessors[primitive.indicesAccessorIndex]["componentType"];
            if (indicesComponentType == 5125) {
                auto indices = glb::Accessor(accessors[primitive.indicesAccessorIndex]).get<uint32_t>(bufferViews, binChunk.data);
                meshPrimitive.indices(indices, flipTriangles);
            }
            else if (indicesComponentType == 5123) {
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

            meshPrimitive.verticesPositions(positions);
            meshPrimitive.verticesUvs(uv1s);

            if (normals.size() != 0) {
                meshPrimitive.verticesNormals(normals);
            }
            else {
                meshPrimitive.computeFlatNormals();
            }

            if (tangents.size() != 0) {
                meshPrimitive.verticesTangents(tangents);
            }
            else {
                meshPrimitive.computeTangents();
            }

            meshPrimitive.material(rmMaterial);
            meshPrimitive.category(translucent ? RenderCategory::Translucent : RenderCategory::Opaque);
        }

        return meshGroup;
    }

    int32_t loadNode(MeshComponent& meshComponent, uint32_t nodeIndex, std::vector<MeshNode>& meshNodes, const glb::Chunk& binChunk,
                       const nlohmann::json& json, CacheData& cacheData)
    {
        glb::Node node(json["nodes"][nodeIndex]);

        if (node.children.empty() && node.meshIndex == -1u) {
            logger.warning("sill.makers.glb-maker")
                << "Node '" << node.name << "' is empty and has no children, so it has been removed." << std::endl;
            return 0;
        }

        // @note We have one MeshNode per glb::Node,
        // and one for each primitive within its mesh (if any).
        cacheData.nodeIndices[nodeIndex] = meshNodes.size();
        meshNodes.emplace_back();
        auto& meshNode = meshNodes.back();
        auto meshNodeIndex = meshNodes.size() - 1u;

        meshNode.name = node.name;
        meshNode.transform(node.transform);

        // @note From https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#transformations
        // > If the determinant of the transform is a negative value,
        // > the winding order of the mesh triangle faces should be reversed.
        // > This supports negative scales for mirroring geometry.
        bool flipTriangles = (glm::determinant(node.transform) < 0);

        // Load geometry if any
        if (node.meshIndex != -1u) {
            meshNode.meshGroup = loadMesh(meshComponent, node.meshIndex, binChunk, json, cacheData, flipTriangles);
        }

        // Recurse over children
        for (auto child : node.children) {
            auto childNodeIndex = loadNode(meshComponent, child, meshNodes, binChunk, json, cacheData);
            if (childNodeIndex != 0) {
                meshNode.children.emplace_back(childNodeIndex - meshNodeIndex);
            }
        }

        return meshNodeIndex;
    }
}

void loadAnimations(MeshComponent& meshComponent, const glb::Chunk& binChunk, const nlohmann::json& json, CacheData& cacheData)
{
    if (json.find("animations") == json.end()) return;

    const auto& accessors = json["accessors"];
    const auto& bufferViews = json["bufferViews"];

    for (auto& animation : json["animations"]) {
        MeshAnimation meshAnimation;

        for (auto& channel : animation["channels"]) {
            uint32_t samplerIndex = channel["sampler"];
            uint32_t nodeIndex = channel["target"]["node"];
            auto& meshChannel = meshAnimation[cacheData.nodeIndices[nodeIndex]].emplace_back();

            uint32_t inputAccessorIndex = animation["samplers"][samplerIndex]["input"];
            uint32_t outputAccessorIndex = animation["samplers"][samplerIndex]["output"];

            // @todo We currently handle just animations expressed in float
            assert(accessors[inputAccessorIndex]["componentType"] == 5126);
            assert(accessors[outputAccessorIndex]["componentType"] == 5126);

            auto input = glb::Accessor(accessors[inputAccessorIndex]).get<float>(bufferViews, binChunk.data);
            input.fill(meshChannel.timeSteps);

            // Path
            const auto& path = channel["target"]["path"];
            if (path == "translation") {
                auto output = glb::Accessor(accessors[outputAccessorIndex]).get<glm::vec3>(bufferViews, binChunk.data);
                output.fill(meshChannel.translation);
                meshChannel.path = MeshAnimationPath::Translation;
            }
            else if (path == "rotation") {
                auto output = glb::Accessor(accessors[outputAccessorIndex]).get<glm::quat>(bufferViews, binChunk.data);
                output.fill(meshChannel.rotation);
                meshChannel.path = MeshAnimationPath::Rotation;
            }
            else if (path == "scale") {
                auto output = glb::Accessor(accessors[outputAccessorIndex]).get<glm::vec3>(bufferViews, binChunk.data);
                output.fill(meshChannel.scaling);
                meshChannel.path = MeshAnimationPath::Scaling;
            }
            else {
                logger.warning("sill.makers.glb-mesh") << "Unhandled animation path: " << path << "." << std::endl;
            }

            // Interpolation
            const auto& interpolation = animation["samplers"][samplerIndex]["interpolation"];
            if (interpolation == "STEP") {
                meshChannel.interpolationType = InterpolationType::Step;
            }
            else if (interpolation == "LINEAR") {
                meshChannel.interpolationType = InterpolationType::Linear;
            }
            else if (interpolation == "CUBICSPLINE") {
                meshChannel.interpolationType = InterpolationType::CubicSpline;
            }
        }

        if (animation.find("name") != animation.end()) {
            std::string name = animation["name"];
            meshComponent.add(name, meshAnimation);
        }
        else {
            logger.warning("sill.makers.glb-mesh") << "Found animation with no name." << std::endl;
        }
    }
}

std::function<void(MeshComponent&)> makers::glbMeshMaker(const std::string& fileName)
{
    logger.info("sill.makers.glb-mesh").tab(3) << "Loading file " << fileName << std::endl;
    logger.log().tab(-3);

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

        meshComponent.path(fileName);

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
            auto meshNodeIndex = loadNode(meshComponent, nodeIndex, meshNodes, binChunk, json, cacheData);
            if (meshNodeIndex != 0) {
                rootNode.children.emplace_back(meshNodeIndex);
            }
        }

        // ----- Animations

        loadAnimations(meshComponent, binChunk, json, cacheData);

        // ----- Fixing convention of glTF (right-handed, front is +X, left is +Z)

        rootNode.transform(rotationMatrixFromAxes(Axis::PositiveZ, Axis::PositiveX, Axis::PositiveY));

        meshComponent.addNodes(std::move(meshNodes));

        // Bind all uniforms to textures once that are all done loaded
        if (cacheData.threadPool) {
            cacheData.threadPool->wait();
        }

        for (const auto& it : cacheData.pendingUniformBindings) {
            auto& texture = cacheData.textures[it.first];
            for (const auto& pair : it.second) {
                pair.first->set(pair.second, texture); // Set material uniform
            }
        }

        logger.info("sill.makers.glb-mesh").tab(3) << "Generated mesh component for " << fileName << std::endl;
        logger.log().tab(-3);
    };
}
