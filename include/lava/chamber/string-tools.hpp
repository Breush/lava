#pragma once

#include <lava/core/u8string.hpp>

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

    // bytesLength will be set to 1, 2, 3 or 4 given the read bytes count.
    // @note Undefined behavior if u is not a valid UTF-8 codepoint start.
    uint32_t utf8Codepoint(const uint8_t* u, uint8_t& bytesLength);
    inline uint32_t utf8Codepoint(const char* u, uint8_t& bytesLength) {
        return utf8Codepoint(reinterpret_cast<const uint8_t*>(u), bytesLength);
    }

    // Return 1, 2, 3 or 4 given the size of the UTF-8 encoding.
    // @note If u is not a valid UTF-8 codepoint start, return 0.
    uint8_t utf8CodepointBytesLength(const uint8_t* u);
    inline uint8_t utf8CodepointBytesLength(const char* u) {
        return utf8CodepointBytesLength(reinterpret_cast<const uint8_t*>(u));
    }

    // Return 1, 2, 3 or 4 given the size of the UTF-8 encoding read backwards.
    // `u - <returnValue>` will therefore be the codepoint start.
    // @note If (u - 1) is not a valid UTF-8 codepoint end, return 0.
    uint8_t utf8CodepointBytesLengthBackwards(const uint8_t* u);
    inline uint8_t utf8CodepointBytesLengthBackwards(const char* u) {
        return utf8CodepointBytesLengthBackwards(reinterpret_cast<const uint8_t*>(u));
    }

    // Convert a unicode codepoint to a UTF-8 encoded string.
    u8string codepointToU8String(uint32_t codepoint);
}
