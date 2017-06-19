#include "./loader.hpp"

using namespace lava::glb;

Image::Image(const typename nlohmann::json::basic_json& json)
{
    bufferView = json["bufferView"];
    mimeType = json["mimeType"];
}

Texture::Texture(const typename nlohmann::json::basic_json& json)
{
    sampler = json["sampler"];
    source = json["source"];
}

PbrMetallicRoughnessMaterial::PbrMetallicRoughnessMaterial(const typename nlohmann::json::basic_json& json)
{
    baseColorTextureIndex = json["pbrMetallicRoughness"]["baseColorTexture"]["index"];
}

Accessor::Accessor(const typename nlohmann::json::basic_json& json)
{
    bufferView = json["bufferView"];
    byteOffset = json["byteOffset"];
    count = json["count"];
}

BufferView::BufferView(const typename nlohmann::json::basic_json& json)
{
    byteLength = json["byteLength"];
    byteOffset = json["byteOffset"];
    if (json.find("byteStride") != json.end()) byteStride = json["byteStride"];
}

std::vector<uint8_t> BufferView::get(const std::vector<uint8_t>& buffer) const
{
    std::vector<uint8_t> vector(byteLength);
    memcpy(vector.data(), &buffer[byteOffset], vector.size());
    return vector;
}
