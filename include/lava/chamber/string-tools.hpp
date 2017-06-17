#pragma once

#include <string>

namespace lava {
    inline std::string operator*(const std::string& word, uint32_t times)
    {
        std::string result;
        result.reserve(times * word.length());
        for (auto a = 0u; a < times; ++a) result += word;
        return result;
    }
}
