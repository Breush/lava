#include <lava/chamber/logger.hpp>

using namespace lava;

Logger lava::logger;

Logger::Logger()
    : m_stream(std::cout, "\e[21m\e[39m")
{
}

LoggerStream& Logger::info(const std::string& category)
{
    m_stream.autoExit(false);
    m_stream << "\e[1m[" << category << "] \e[32m";
    return m_stream;
}

LoggerStream& Logger::warning(const std::string& category)
{
    m_stream.autoExit(false);
    m_stream << "\e[1m[" << category << "] \e[33m/!\\ ";
    return m_stream;
}

LoggerStream& Logger::error(const std::string& category)
{
    m_stream.autoExit(true);
    m_stream << "\e[1m[" << category << "] \e[31m/!\\ /!\\ ";
    return m_stream;
}
