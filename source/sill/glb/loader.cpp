#include "./loader.hpp"

using namespace lava;
using namespace lava::glb;

Node::Node(const typename nlohmann::json::basic_json& json)
{
    if (json.find("name") != json.end()) name = json["name"];
    if (json.find("mesh") != json.end()) meshIndex = json["mesh"];

    if (json.find("children") != json.end()) {
        for (const auto& child : json["children"]) {
            children.emplace_back(child);
        }
    }

    transform = glm::mat4(1.f);

    if (json.find("matrix") != json.end()) {
        transform[0u][0u] = json["matrix"][0u];
        transform[0u][1u] = json["matrix"][1u];
        transform[0u][2u] = json["matrix"][2u];
        transform[0u][3u] = json["matrix"][3u];
        transform[1u][0u] = json["matrix"][4u];
        transform[1u][1u] = json["matrix"][5u];
        transform[1u][2u] = json["matrix"][6u];
        transform[1u][3u] = json["matrix"][7u];
        transform[2u][0u] = json["matrix"][8u];
        transform[2u][1u] = json["matrix"][9u];
        transform[2u][2u] = json["matrix"][10u];
        transform[2u][3u] = json["matrix"][11u];
        transform[3u][0u] = json["matrix"][12u];
        transform[3u][1u] = json["matrix"][13u];
        transform[3u][2u] = json["matrix"][14u];
        transform[3u][3u] = json["matrix"][15u];
    }
    else {
        if (json.find("translation") != json.end()) {
            glm::vec3 translation;
            translation.x = json["translation"][0u];
            translation.y = json["translation"][1u];
            translation.z = json["translation"][2u];
            transform = glm::translate(transform, translation);
        }

        if (json.find("rotation") != json.end()) {
            glm::quat rotation;
            rotation.x = json["rotation"][0u];
            rotation.y = json["rotation"][1u];
            rotation.z = json["rotation"][2u];
            rotation.w = json["rotation"][3u];
            transform = glm::mat4(rotation) * transform;
        }

        if (json.find("scale") != json.end()) {
            glm::vec3 scale;
            scale.x = json["scale"][0u];
            scale.y = json["scale"][1u];
            scale.z = json["scale"][2u];
            transform = glm::scale(transform, scale);
        }
    }
}

Mesh::Mesh(const typename nlohmann::json::basic_json& json)
{
    if (json.find("name") != json.end()) name = json["name"];

    for (auto primitiveIndex = 0u; primitiveIndex < json["primitives"].size(); ++primitiveIndex) {
        primitives.emplace_back();
        auto& primitiveData = primitives.back();

        const auto& primitive = json["primitives"][primitiveIndex];
        if (primitive.find("mode") != primitive.end()) primitiveData.mode = primitive["mode"];

        const auto& attributes = primitive["attributes"];
        primitiveData.positionsAccessorIndex = attributes["POSITION"];
        if (attributes.find("NORMAL") != attributes.end()) primitiveData.normalsAccessorIndex = attributes["NORMAL"];
        if (attributes.find("TANGENT") != attributes.end()) primitiveData.tangentsAccessorIndex = attributes["TANGENT"];
        if (attributes.find("TEXCOORD_0") != attributes.end()) primitiveData.uv1sAccessorIndex = attributes["TEXCOORD_0"];

        primitiveData.indicesAccessorIndex = primitive["indices"];
        if (primitive.find("material") != primitive.end()) primitiveData.materialIndex = primitive["material"];
    }
}

Image::Image(const typename nlohmann::json::basic_json& json)
{
    bufferView = json["bufferView"];
    mimeType = json["mimeType"];
}

Texture::Texture(const typename nlohmann::json::basic_json& json)
{
    if (json.find("sampler") != json.end()) sampler = json["sampler"];
    source = json["source"];
}

Material::Material(const typename nlohmann::json::basic_json& json)
{
    if (json.find("normalTexture") != json.end()) normalTextureIndex = json["normalTexture"]["index"];
    if (json.find("occlusionTexture") != json.end()) occlusionTextureIndex = json["occlusionTexture"]["index"];
    if (json.find("emissiveTexture") != json.end()) emissiveTextureIndex = json["emissiveTexture"]["index"];
}

PbrMetallicRoughnessMaterial::PbrMetallicRoughnessMaterial(const typename nlohmann::json::basic_json& json)
    : Material(json)
{
    if (json.find("alphaMode") != json.end()) {
        if (json["alphaMode"] != "OPAQUE") {
            translucent = true;
        }
    }

    if (json.find("pbrMetallicRoughness") == json.end()) return;
    auto& pbrMetallicRoughness = json["pbrMetallicRoughness"];

    if (pbrMetallicRoughness.find("baseColorTexture") != pbrMetallicRoughness.end()) {
        baseColorTextureIndex = pbrMetallicRoughness["baseColorTexture"]["index"];
    }
    if (pbrMetallicRoughness.find("metallicRoughnessTexture") != pbrMetallicRoughness.end()) {
        metallicRoughnessTextureIndex = pbrMetallicRoughness["metallicRoughnessTexture"]["index"];
    }
    if (pbrMetallicRoughness.find("baseColorFactor") != pbrMetallicRoughness.end()) {
        baseColorFactor.r = pbrMetallicRoughness["baseColorFactor"][0];
        baseColorFactor.g = pbrMetallicRoughness["baseColorFactor"][1];
        baseColorFactor.b = pbrMetallicRoughness["baseColorFactor"][2];
        baseColorFactor.a = pbrMetallicRoughness["baseColorFactor"][3];
    }
}

Accessor::Accessor(const typename nlohmann::json::basic_json& json)
{
    if (json.find("bufferView") != json.end()) bufferView = json["bufferView"];
    if (json.find("byteOffset") != json.end()) byteOffset = json["byteOffset"];
    count = json["count"];
}

BufferView::BufferView(const typename nlohmann::json::basic_json& json)
{
    byteLength = json["byteLength"];
    if (json.find("byteOffset") != json.end()) byteOffset = json["byteOffset"];
    if (json.find("byteStride") != json.end()) byteStride = json["byteStride"];
}

VectorView<uint8_t> BufferView::get(const std::vector<uint8_t>& buffer) const
{
    auto size = (byteStride != 0u) ? byteLength / byteStride : byteLength;
    return VectorView<uint8_t>(buffer.data() + byteOffset, size, byteStride);
}
