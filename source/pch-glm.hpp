#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>

inline std::ostream& operator<<(std::ostream& os, const glm::vec3& v)
{
    return os << glm::to_string(v);
}

inline std::ostream& operator<<(std::ostream& os, const glm::vec4& v)
{
    return os << glm::to_string(v);
}

inline std::ostream& operator<<(std::ostream& os, const glm::quat& q)
{
    return os << glm::to_string(q);
}

inline std::ostream& operator<<(std::ostream& os, const glm::mat4& m)
{
    return os << glm::to_string(m);
}
