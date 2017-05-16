#include <lava/chamber/logger-stream.hpp>

using namespace lava;

LoggerStream::LoggerStream(std::ostream& stream, const std::string& resetString)
    : m_stream(&stream)
    , m_resetString(resetString)
{
}

LoggerStream& LoggerStream::operator[](uint8_t i)
{
    if (i == 1) (*m_stream) << "    ";
    if (i == 2) (*m_stream) << "        ";
    if (i == 3) (*m_stream) << "            ";
    return *this;
}

LoggerStream& LoggerStream::operator<<(const std::string& string)
{
    (*m_stream) << string;
    return *this;
}

LoggerStream& LoggerStream::operator<<(int64_t number)
{
    (*m_stream) << number;
    return *this;
}

LoggerStream& LoggerStream::operator<<(uint64_t number)
{
    (*m_stream) << number;
    return *this;
}

LoggerStream& LoggerStream::operator<<(LoggerStream::EndlType endl)
{
    (*m_stream) << m_resetString << endl;
    return *this;
}
