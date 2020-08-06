#include "./i-call-stack-impl.hpp"

#include <lava/chamber/string-tools.hpp>

using namespace lava::chamber;

std::string ICallStackImpl::toString() const
{
    std::ostringstream os;

    // Find spacing
    uint64_t spacingLength = 0u;
    for (auto& entry : m_entries) {
        std::stringstream str;
        str << entry.file << ":" << entry.line;
        spacingLength = std::max(spacingLength, str.str().size());
    }

    // Print
    for (auto& entry : m_entries) {
        std::stringstream str;
        str << entry.file << ":" << entry.line;
        const auto spacing = std::string(" ") * (spacingLength - str.str().size() + 1);
        os << "\e[1m" << entry.file << "\e[2m:" << entry.line << "\e[0m" << spacing;

        // Parse and color the function
        const auto leftParenthesis = entry.function.find_first_of('(');
        auto functionNameColon = entry.function.find_last_of(':', leftParenthesis) + 1u;
        if (functionNameColon == std::string::npos) functionNameColon = 0u;

        std::string functionPrefix = entry.function.substr(0u, functionNameColon);
        std::string functionName = entry.function.substr(functionNameColon, leftParenthesis - functionNameColon);
        std::string arguments;
        if (leftParenthesis != std::string::npos) {
            arguments = entry.function.substr(leftParenthesis);
        }

        os << functionPrefix << "\e[94m" << functionName;
        if (!arguments.empty()) {
            os << "\e[37m" << arguments;
        }
        os << "\e[0m" << std::endl;
    }

    return os.str();
}
