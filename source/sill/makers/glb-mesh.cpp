#include <lava/sill/makers/glb-mesh.hpp>

#include <fstream>
#include <glm/glm.hpp>
#include <lava/chamber/logger.hpp>
// @todo Should be defined in sill via a abstracted interface for materials (with no knowledge of vulkan's internals)
#include <lava/magma/materials/rm-material.hpp>
#include <stb/stb_image.h>

#include "../components/mesh-component-impl.hpp"
#include "../game-engine-impl.hpp"
#include "../game-entity-impl.hpp"
#include "../glb/loader.hpp"

using namespace lava::chamber;
using namespace lava::sill;

std::function<void(MeshComponent& meshComponent)> makers::glbMeshMaker(const std::string& fileName)
{
    return [&fileName](MeshComponent& meshComponent) {
        logger.info("sill.makers.glb-mesh") << "Loading file " << fileName << std::endl;

        std::ifstream file(fileName, std::ifstream::binary);

        if (!file.is_open()) {
            logger.error("sill.makers.glb-mesh") << "Unable to read file " << fileName << std::endl;
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
            logger.error("sill.makers.glb-mesh") << "Invalid mode " << primitive["mode"] << " for primitive."
                                                 << " Only 4 (TRIANGLES) is supported." << std::endl;
            return;
        }

        // Positions
        if (attributes.find("POSITION") == attributes.end()) {
            logger.error("sill.makers.glb-mesh") << "No position found." << std::endl;
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
        std::vector<glm::vec3> normals;
        if (attributes.find("NORMAL") != attributes.end()) {
            uint32_t normalsAccessorIndex = attributes["NORMAL"];
            glb::Accessor normalsAccessor(accessors[normalsAccessorIndex]);
            normals = normalsAccessor.get<glm::vec3>(bufferViews, binChunk.data);

            // Fixing axes conventions
            for (auto& v : normals) {
                auto z = v.z;
                v.z = v.y;
                v.y = -z;
            }
        }
        else {
            // @todo Compute flat normals
            logger.error("sill.makers.glb-mesh") << "No normals found. Currently not generating flat normals." << std::endl;
        }

        // Tangents
        std::vector<glm::vec4> tangents;
        if (attributes.find("TANGENT") != attributes.end()) {
            uint32_t tangentsAccessorIndex = attributes["TANGENT"];
            glb::Accessor tangentsAccessor(accessors[tangentsAccessorIndex]);
            tangents = tangentsAccessor.get<glm::vec4>(bufferViews, binChunk.data);

            // @todo Don't we need to fix axes conventions?
        }
        else {
            // @todo Otherwise, auto compute tangents nicely
            for (auto& v : normals) {
                tangents.emplace_back(-v.y, v.x, v.z, 1);
            }
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
        auto& rmMaterial = meshComponent.impl().entity().engine().renderScene().make<magma::RmMaterial>();

        // Material textures
        if (material.baseColorTextureIndex != -1u) {
            uint32_t textureIndex = material.baseColorTextureIndex;
            glb::Texture texture(textures[textureIndex]);
            glb::Image image(images[texture.source]);

            int texWidth, texHeight;
            glb::BufferView imageBufferView(bufferViews[image.bufferView]);
            auto imageData = imageBufferView.get(binChunk.data);
            auto pixels =
                stbi_load_from_memory(imageData.data(), imageData.size(), &texWidth, &texHeight, nullptr, STBI_rgb_alpha);
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
            auto pixels =
                stbi_load_from_memory(imageData.data(), imageData.size(), &texWidth, &texHeight, nullptr, STBI_rgb_alpha);
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
            auto pixels =
                stbi_load_from_memory(imageData.data(), imageData.size(), &texWidth, &texHeight, nullptr, STBI_rgb_alpha);
            std::vector<uint8_t> pixelsVector(texWidth * texHeight * 4);
            memmove(pixelsVector.data(), pixels, pixelsVector.size());
            rmMaterial.metallicRoughnessColor(pixelsVector, texWidth, texHeight, 4);
            stbi_image_free(pixels);
        }

        meshComponent.verticesCount(positions.size());
        meshComponent.verticesPositions(positions);
        meshComponent.verticesNormals(normals);
        meshComponent.verticesTangents(tangents);
        meshComponent.verticesUvs(uv1s);
        meshComponent.indices(indices);
        meshComponent.material(rmMaterial);
    };
}
