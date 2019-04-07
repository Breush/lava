#pragma once

inline std::string operator*(const std::string& word, uint32_t times)
{
    std::string result;
    result.reserve(times * word.length());
    for (auto a = 0u; a < times; ++a) result += word;
    return result;
}

namespace lava::chamber {
    /// Convert a CamelCaseString to a snake-case-string.
    std::string camelToSnakeCase(const std::string& camelCaseString, std::string separator = "-");

    /// Split a string into a vector of string views.
    std::vector<std::wstring_view> splitAsViews(const std::wstring& s, wchar_t c);

    /// Split a string into a vector of strings.
    std::vector<std::string> split(const std::string& s, char c);
}
