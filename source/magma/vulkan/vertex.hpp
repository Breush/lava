#pragma once

namespace lava::magma::vulkan {
    class UnlitVertex {
    public:
        glm::vec3 pos;
    };

    class Vertex {
    public:
        glm::vec3 pos;
        glm::vec2 uv;
        glm::vec3 normal;
        glm::vec4 tangent;
    };
}
