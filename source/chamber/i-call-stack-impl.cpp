#include "./i-call-stack-impl.hpp"

#include <cstdlib>
#include <lava/chamber/string-tools.hpp>
#include <sstream>

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
        os << "\e[1m" << entry.file << "\e[2m:" << entry.line << "\e[0m" << spacing << entry.function << std::endl;
    }

    return os.str();
}
