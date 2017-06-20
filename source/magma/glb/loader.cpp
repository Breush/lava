#include "./loader.hpp"

using namespace lava::glb;

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

PbrMetallicRoughnessMaterial::PbrMetallicRoughnessMaterial(const typename nlohmann::json::basic_json& json)
{
    if (json.find("pbrMetallicRoughness") != json.end()) {
        auto& pbrMetallicRoughness = json["pbrMetallicRoughness"];
        if (pbrMetallicRoughness.find("baseColorTexture") != pbrMetallicRoughness.end()) {
            baseColorTextureIndex = pbrMetallicRoughness["baseColorTexture"]["index"];
        }
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

std::vector<uint8_t> BufferView::get(const std::vector<uint8_t>& buffer) const
{
    std::vector<uint8_t> vector(byteLength);
    memcpy(vector.data(), &buffer[byteOffset], vector.size());
    return vector;
}
