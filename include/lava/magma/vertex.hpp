#pragma once

namespace lava::magma {
    class UnlitVertex {
    public:
        glm::vec3 pos;
    };

    class Vertex {
    public:
        glm::vec3 pos = glm::vec3(0);
        glm::vec2 uv = glm::vec2(0);
        glm::vec3 normal = glm::vec3(0);
        glm::vec4 tangent = glm::vec4(1, 0, 0, 1);
    };

    class FlatVertex {
    public:
        glm::vec2 pos = glm::vec2(0);
        glm::vec2 uv = glm::vec2(0);
    };
}
