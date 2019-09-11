#pragma once

namespace lava::sill {
    void addCirclePoints(std::vector<glm::vec3>& points, std::vector<glm::vec2>& uvs, const uint32_t tessellation,
                         const float sphereRadius, const float radius, const float height);

    void addPoleStrip(std::vector<uint16_t>& indices, uint32_t tessellation, uint16_t startIndex, uint16_t poleIndex,
                      bool reverse);

    void addRowStrip(std::vector<uint16_t>& indices, uint32_t tessellation, uint16_t startIndex);
}
