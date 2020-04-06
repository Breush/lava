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

std::string chamber::camelToSnakeCase(const std::string& camelCaseString, std::string separator)
{
    if (camelCaseString.empty()) return "";

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
    if (spacing != nullptr) *spacing = "";
    while (offset < line.size() && std::isspace(line[offset])) {
        if (spacing != nullptr) *spacing += line[offset];
        offset += 1u;
    }

    // Extract word
    word = "";
    while (offset < line.size() && !std::isspace(line[offset])) {
        word += line[offset];
        offset += 1u;
    }

    return offset;
}
