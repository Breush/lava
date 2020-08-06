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
    std::string camelToSnakeCase(const std::string& camelCaseString, const std::string& separator = "-");

    /// Split a string into a vector of string views.
    std::vector<std::string_view> splitAsViews(const std::string& s, char c);
    std::vector<std::wstring_view> splitAsViews(const std::wstring& s, wchar_t c);

    /// Split a string into a vector of strings.
    std::vector<std::string> split(const std::string& s, char c);

    /// Extract next word (surrounded by whitespace-like characters) from a "line".
    uint32_t nextWord(const std::string& line, std::string& word, uint32_t offset = 0u, std::string* spacing = nullptr);

    /// Convert a UTF16 string to UTF8.
    std::string utf16to8(const std::wstring& ws);

    /// Convert a UTF8 string to UTF16.
    std::wstring utf8to16(const std::string& s);
}
