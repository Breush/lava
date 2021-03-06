#include <lava/chamber/logger.hpp>

#include <lava/chamber/math.hpp>
#include <lava/chamber/string-tools.hpp>

using namespace lava;

chamber::Logger chamber::logger;

namespace {
    std::string spacing(const std::string& string)
    {
        static const std::string space(" ");
        static uint32_t maxStringLength = 0u;
        uint32_t stringLength = string.size();
        maxStringLength = std::max(maxStringLength, stringLength);
        return space * (maxStringLength - stringLength);
    }
}

using namespace lava::chamber;

Logger::Logger()
    : m_stream(std::cout, "\e[0m")
{
}

LoggerStream& Logger::log()
{
    return m_stream;
}

LoggerStream& Logger::info(const std::string& category)
{
    m_stream.kind(LoggerKind::Info);
    m_stream.autoExit(false);
    m_stream.prefixString("\e[1m[" + category + "] \e[32m" + spacing(category));
    return m_stream;
}

LoggerStream& Logger::warning(const std::string& category)
{
    static const std::string follow("/!\\ ");
    m_stream.kind(LoggerKind::Unknown);
    m_stream.autoExit(false);
    m_stream.prefixString("\e[1m[" + category + "] \e[33m" + follow + spacing(category + follow));
    return m_stream;
}

LoggerStream& Logger::error(const std::string& category)
{
    static const std::string follow("//!\\\\ ");
    m_stream.kind(LoggerKind::Unknown);
    m_stream.autoExit(true);
    m_stream.prefixString("\e[1m[" + category + "] \e[31m" + follow + spacing(category + follow));
    return m_stream;
}
