#include "./makers-common.hpp"

using namespace lava;
using namespace lava::chamber;

void sill::addCirclePoints(std::vector<glm::vec3>& points, std::vector<glm::vec2>& uvs, const uint32_t tessellation,
                           const float sphereRadius, const float radius, const float height)
{
    const auto uvStep = 1.f / tessellation;
    const auto step = math::TWO_PI / tessellation;
    const auto cStep = math::cos(step);
    const auto sStep = math::sin(step);

    glm::vec3 point{radius, 0.f, height};
    glm::vec2 uv{0.f, -height / sphereRadius * 0.5f + 0.5f};
    for (auto j = 0u; j < tessellation; ++j) {
        points.emplace_back(point);
        const auto px = point.x;
        point.x = px * cStep - point.y * sStep;
        point.y = px * sStep + point.y * cStep;

        uvs.emplace_back(uv);
        uv.x += uvStep;
    }

    // Last point is the first one with custom uvs
    points.emplace_back(radius, 0.f, height);
    uvs.emplace_back(1.f, uv.y);
}

void sill::addPoleStrip(std::vector<uint16_t>& indices, uint32_t tessellation, uint16_t startIndex, uint16_t poleIndex,
                        bool reverse)
{
    for (auto i = 0u; i < tessellation; i++) {
        uint16_t d0 = startIndex + i;
        uint16_t d1 = d0 + 1u;
        if (reverse) std::swap(d0, d1);
        indices.emplace_back(d0);
        indices.emplace_back(d1);
        indices.emplace_back(poleIndex);
    }
}

void sill::addRowStrip(std::vector<uint16_t>& indices, uint32_t tessellation, uint16_t startIndex)
{
    for (auto i = 0u; i < tessellation; i++) {
        uint16_t d0 = startIndex + i;
        uint16_t d1 = d0 + 1u;
        uint16_t u0 = d0 + tessellation + 1u;
        uint16_t u1 = d1 + tessellation + 1u;
        indices.emplace_back(d0);
        indices.emplace_back(d1);
        indices.emplace_back(u1);
        indices.emplace_back(u1);
        indices.emplace_back(u0);
        indices.emplace_back(d0);
    }
}
