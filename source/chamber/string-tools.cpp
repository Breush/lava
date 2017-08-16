#include <lava/chamber/string-tools.hpp>

#include <sstream>

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
