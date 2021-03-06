#include <lava/sill/makers/glb-mesh.hpp>

#include <lava/sill/i-mesh.hpp>

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
        std::unordered_map<uint32_t, RenderCategory> renderCategories;
        std::unordered_map<uint32_t, uint32_t> nodeIndices;

        // Key is textureId, value is a list of materials and uniformName waiting for that texture to be done.
        std::unordered_map<uint32_t, std::vector<std::pair<magma::Material*, std::string>>> pendingUniformBindings;
    };

    using PixelsCallback = std::function<void(uint8_t*, uint32_t, uint32_t)>;

    void setTexture(magma::RenderEngine& renderEngine, magma::Material& material, const std::string& uniformName, uint32_t textureIndex,
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

        cacheData.threadPool->job([&renderEngine, &cacheData, textureIndex, imageVectorView, pixelsCallback] {
            // @todo We might want to choose the number of channels one day...
            int texWidth, texHeight;
            auto pixels = stbi_load_from_memory(imageVectorView.data(), imageVectorView.size(), &texWidth, &texHeight, nullptr,
                                                STBI_rgb_alpha);
            if (pixelsCallback) pixelsCallback(pixels, texWidth, texHeight);

            // @note Because this is threaded, a badly-made GLB can ask to load the same texture multiple times
            // if the data is duplicated. We cannot do anything at this level...
            // This findTexture is for already loaded textures from other meshes.
            auto existingTexture = renderEngine.findTexture(pixels, texWidth, texHeight, 4u);
            if (existingTexture != nullptr) {
                cacheData.textures[textureIndex] = std::move(existingTexture);
            }
            else {
                auto texture = renderEngine.makeTexture();
                texture->loadFromMemory(pixels, texWidth, texHeight, 4u);
                cacheData.textures[textureIndex] = std::move(texture);
            }

            stbi_image_free(pixels);
        });
    }

    void setOrmTexture(magma::RenderEngine& renderEngine, magma::Material& material, uint32_t occlusionTextureIndex,
                       uint32_t metallicRoughnessTextureIndex, const glb::Chunk& binChunk, const nlohmann::json& json,
                       CacheData& cacheData)
    {
        // Occlusion missing, forcing it to 255u.
        if (occlusionTextureIndex == -1u && metallicRoughnessTextureIndex != -1u) {
            setTexture(renderEngine, material, "roughnessMetallicMap", metallicRoughnessTextureIndex, binChunk, json, cacheData,
                       [](uint8_t* pixels, uint32_t width, uint32_t height) {
                           auto length = 4u * width * height;
                           for (auto i = 0u; i < length; i += 4u) {
                               pixels[i] = 255u;
                           }
                       });
            return;
        }

        // Every other case is valid.
        setTexture(renderEngine, material, "occlusionMap", occlusionTextureIndex, binChunk, json, cacheData);
        setTexture(renderEngine, material, "roughnessMetallicMap", metallicRoughnessTextureIndex, binChunk, json, cacheData);
    }

    void loadMesh(IMesh& iMesh, uint32_t iMeshNodeIndex, uint32_t meshIndex, const glb::Chunk& binChunk,
                  const nlohmann::json& json, CacheData& cacheData, bool flipTriangles)
    {
        PROFILE_FUNCTION();

        glb::Mesh mesh(json["meshes"][meshIndex]);

        auto& scene = iMesh.scene();
        auto& renderEngine = scene.engine();
        const auto& accessors = json["accessors"];
        const auto& bufferViews = json["bufferViews"];
        const auto& materials = json.find("materials");

        auto& group = iMesh.nodeMakeGroup(iMeshNodeIndex);
        group.name(mesh.name);

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
            RenderCategory renderCategory = RenderCategory::Opaque;
            magma::MaterialPtr rmMaterial = nullptr;
            if (cacheData.materials.find(primitive.materialIndex) != cacheData.materials.end()) {
                rmMaterial = cacheData.materials.at(primitive.materialIndex);
                renderCategory = cacheData.renderCategories.at(primitive.materialIndex);
            }
            else if (materials != json.end() && primitive.materialIndex != -1u) {
                // @note We need "roughness-metallic" material.
                if (renderEngine.materialInfoIfExists("roughness-metallic") == nullptr) {
                    renderEngine.registerMaterialFromFile("roughness-metallic", "./data/shaders/materials/rm-material.shmag");
                }

                glb::PbrMetallicRoughnessMaterial material((*materials)[primitive.materialIndex]);
                rmMaterial = scene.makeMaterial("roughness-metallic");
                rmMaterial->name(material.name);
                renderCategory = material.renderCategory;

                // Material textures
                rmMaterial->set("albedoColor", material.baseColorFactor);
                rmMaterial->set("metallicFactor", material.metallicFactor);
                rmMaterial->set("roughnessFactor", material.roughnessFactor);
                setTexture(renderEngine, *rmMaterial, "albedoMap", material.baseColorTextureIndex, binChunk, json, cacheData);
                setTexture(renderEngine, *rmMaterial, "normalMap", material.normalTextureIndex, binChunk, json, cacheData);
                setTexture(renderEngine, *rmMaterial, "emissiveMap", material.emissiveTextureIndex, binChunk, json, cacheData);
                setOrmTexture(renderEngine, *rmMaterial, material.occlusionTextureIndex, material.metallicRoughnessTextureIndex,
                              binChunk, json, cacheData);

                cacheData.materials[primitive.materialIndex] = rmMaterial;
                cacheData.renderCategories[primitive.materialIndex] = renderCategory;
            }

            // Apply the geometry
            auto& meshPrimitive = group.addPrimitive();
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
            meshPrimitive.renderCategory(renderCategory);
        }
    }

    uint32_t loadNode(IMesh& iMesh, uint32_t nodeIndex, const glb::Chunk& binChunk,
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
        auto iMeshNodeIndex = iMesh.addNode();
        cacheData.nodeIndices[nodeIndex] = iMeshNodeIndex;

        iMesh.nodeName(iMeshNodeIndex, node.name);
        iMesh.nodeMatrix(iMeshNodeIndex, node.transform);

        // @note From https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#transformations
        // > If the determinant of the transform is a negative value,
        // > the winding order of the mesh triangle faces should be reversed.
        // > This supports negative scales for mirroring geometry.
        bool flipTriangles = (glm::determinant(node.transform) < 0);

        // Load geometry if any
        if (node.meshIndex != -1u) {
            loadMesh(iMesh, iMeshNodeIndex, node.meshIndex, binChunk, json, cacheData, flipTriangles);
        }

        // Recurse over children
        for (auto child : node.children) {
            auto iMeshChildNodeIndex = loadNode(iMesh, child, binChunk, json, cacheData);
            if (iMeshNodeIndex != 0) {
                iMesh.nodeAddAbsoluteChild(iMeshNodeIndex, iMeshChildNodeIndex);
            }
        }

        return iMeshNodeIndex;
    }
}

void loadAnimations(IMesh& iMesh, const glb::Chunk& binChunk, const nlohmann::json& json, CacheData& cacheData)
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
            iMesh.addAnimation(name, meshAnimation);
        }
        else {
            logger.warning("sill.makers.glb-mesh") << "Found animation with no name." << std::endl;
        }
    }
}

std::function<uint32_t(IMesh&)> makers::glbMeshMaker(const std::string& fileName)
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

    return [=](IMesh& iMesh) -> uint32_t {
        PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

        iMesh.path(fileName);

        // ----- Resources pre-allocation

        // Caching
        CacheData cacheData;

        // The mesh nodes array
        iMesh.reserveNodes(json["nodes"].size() + 1u);

        // ----- Scene loading

        uint32_t rootScene = json["scene"];
        auto nodes = json["scenes"][rootScene]["nodes"];

        auto iMeshRootNodeIndex = iMesh.addNode();
        iMesh.nodeName(iMeshRootNodeIndex, fileName);

        for (uint32_t nodeIndex : nodes) {
            auto iMeshNodeIndex = loadNode(iMesh, nodeIndex, binChunk, json, cacheData);
            if (iMeshNodeIndex != 0) {
                iMesh.nodeAddAbsoluteChild(iMeshRootNodeIndex, iMeshNodeIndex);
            }
        }

        // ----- Animations

        loadAnimations(iMesh, binChunk, json, cacheData);

        // ----- Fixing convention of glTF (right-handed, front is +X, left is +Z)

        iMesh.nodeMatrix(iMeshRootNodeIndex, rotationMatrixFromAxes(Axis::PositiveZ, Axis::PositiveX, Axis::PositiveY));

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

        return iMeshRootNodeIndex;
    };
}
