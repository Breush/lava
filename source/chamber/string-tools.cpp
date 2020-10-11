#include <lava/chamber/string-tools.hpp>

namespace {
    template <class Output, class String, class Character>
    inline std::vector<Output> generalSplit(const String& s, Character c)
    {
        std::vector<Output> views;
        Output view(s);

        while (!view.empty()) {
            auto offset = view.find(c);
            if (offset == std::string::npos) {
                views.emplace_back(view);
                break;
            }
            views.emplace_back(view.substr(0u, offset));
            view = view.substr(offset + 1u);
        }

        return views;
    }
}

using namespace lava;

std::string chamber::camelToSnakeCase(const std::string& camelCaseString, const std::string& separator)
{
    if (camelCaseString.empty()) return std::string();

    std::stringstream snakeCaseStream;

    // First letter is just lowered
    snakeCaseStream << static_cast<char>(std::tolower(camelCaseString[0]));

    // Other letters are separated before lowering
    for (auto i = 1u; i < camelCaseString.size(); ++i) {
        if (camelCaseString[i] >= 'A' && camelCaseString[i] <= 'Z') {
            snakeCaseStream << separator;
        }
        snakeCaseStream << static_cast<char>(std::tolower(camelCaseString[i]));
    }

    return snakeCaseStream.str();
}

std::vector<std::wstring_view> chamber::splitAsViews(const std::wstring& s, wchar_t c)
{
    return generalSplit<std::wstring_view>(s, c);
}

std::vector<std::string_view> chamber::splitAsViews(const std::string& s, char c)
{
    return generalSplit<std::string_view>(s, c);
}

std::vector<std::string> chamber::split(const std::string& s, char c)
{
    return generalSplit<std::string>(s, c);
}

uint32_t chamber::nextWord(const std::string& line, std::string& word, uint32_t offset, std::string* spacing)
{
    // Get spacing before word (if any)
    if (spacing != nullptr) spacing->clear();
    while (offset < line.size() && std::isspace(line[offset])) {
        if (spacing != nullptr) *spacing += line[offset];
        offset += 1u;
    }

    // Extract word
    word.clear();
    while (offset < line.size() && !std::isspace(line[offset])) {
        word += line[offset];
        offset += 1u;
    }

    return offset;
}

std::string chamber::utf16to8(const std::wstring& ws)
{
    using ConvertType = std::codecvt_utf8_utf16<wchar_t>;
    std::wstring_convert<ConvertType, wchar_t> converter;
    return converter.to_bytes(ws);
}

std::wstring chamber::utf8to16(const std::string& s)
{
    using ConvertType = std::codecvt_utf8_utf16<wchar_t>;
    std::wstring_convert<ConvertType, wchar_t> converter;
    return converter.from_bytes(s);
}

uint32_t chamber::utf8Codepoint(const uint8_t* u, uint8_t& bytesLength)
{
    uint8_t u0 = u[0];
    if (u0 <= 127) {
        bytesLength = 1;
        return u0;
    }

    uint8_t u1 = u[1];
    if (u0 >= 192 && u0 <= 223) {
        bytesLength = 2;
        return (u0 - 192) * 64 + (u1 - 128);
    }

    uint8_t u2 = u[2];
    if (u0 >= 224 && u0 <= 239) {
        bytesLength = 3;
        return (u0 - 224) * 4096 + (u1 - 128) * 64 + (u2 - 128);
    }

    uint8_t u3 = u[3];
    bytesLength = 4;
    return (u0 - 240) * 262144 + (u1 - 128) * 4096 + (u2 - 128) * 64 + (u3 - 128);
}

uint8_t chamber::utf8CodepointBytesLength(const uint8_t* u)
{
    uint8_t u0 = u[0];
    if (0b0000'0000 == (u0 & 0b1000'0000)) return 1;
    if (0b1100'0000 == (u0 & 0b1110'0000)) return 2;
    if (0b1110'0000 == (u0 & 0b1111'0000)) return 3;
    if (0b1111'0000 == (u0 & 0b1111'1000)) return 4;
    return 0;
}

uint8_t chamber::utf8CodepointBytesLengthBackwards(const uint8_t* u)
{
    uint8_t um1 = u[-1];
    if (0b0000'0000 == (um1 & 0b1000'0000)) return 1;
    if (0b1000'0000 != (um1 & 0b1100'0000)) return 0;

    uint8_t um2 = u[-2];
    if (0b1100'0000 == (um2 & 0b1110'0000)) return 2;
    if (0b1000'0000 != (um2 & 0b1100'0000)) return 0;

    uint8_t um3 = u[-3];
    if (0b1110'0000 == (um3 & 0b1111'0000)) return 3;
    if (0b1000'0000 != (um3 & 0b1100'0000)) return 0;

    uint8_t um4 = u[-4];
    if (0b1111'0000 == (um4 & 0b1111'1000)) return 4;

    return 0;
}

u8string chamber::codepointToU8String(uint32_t codepoint)
{
    char c[5] = {0};

    if (codepoint <= 0x7F) {
        c[0] = codepoint;
    }
    else if (codepoint <= 0x7FF) {
        c[0] = (codepoint >> 6) + 192;
        c[1] = (codepoint & 63) + 128;
    }
    else if (0xD800 <= codepoint && codepoint <= 0xDfff) {
        // Invalid UTF-8
    }
    else if (codepoint <= 0xFFFF) {
        c[0] = (codepoint >> 12) + 224;
        c[1] = ((codepoint >> 6) & 63) + 128;
        c[2] = (codepoint & 63) + 128;
    }
    else if (codepoint <= 0x10FFFF) {
        c[0] = (codepoint >> 18) + 240;
        c[1] = ((codepoint >> 12) & 63) + 128;
        c[2] = ((codepoint >> 6) & 63) + 128;
        c[3] = (codepoint & 63) + 128;
    }

    return u8string(c);
};
