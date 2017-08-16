#pragma once

#include <string>

inline std::string operator*(const std::string& word, uint32_t times)
{
    std::string result;
    result.reserve(times * word.length());
    for (auto a = 0u; a < times; ++a) result += word;
    return result;
}

namespace lava::chamber {
    /// Convert a CamelCaseString to a snake-case-string
    std::string camelToSnakeCase(const std::string& camelCaseString, std::string separator = "-");
}
