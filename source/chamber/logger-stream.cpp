#include <lava/chamber/logger-stream.hpp>

#include <lava/chamber/string-tools.hpp>

using namespace lava::chamber;

LoggerStream::LoggerStream(std::ostream& stream, const std::string& resetString)
    : std::ostream(this)
    , m_stream(&stream)
    , m_resetString(resetString)
{
}

LoggerStream& LoggerStream::operator[](uint8_t i)
{
    const std::string tab("    ");
    if (i == 1) (*this) << tab * i;
    return *this;
}

LoggerStream& LoggerStream::tab(int8_t i)
{
    const std::string tab("    ");
    if (i > 0) {
        m_prefixString += tab * i;
    }
    else if (i < 0) {
        m_prefixString = m_prefixString.substr(0, m_prefixString.length() + tab.size() * i);
    }
    return *this;
}

int LoggerStream::overflow(int c)
{
    if (c == '\n') {
        stream() << resetString();
        m_beenReset = true;

        if (m_autoExit) {
            CallStack callStack;
            callStack.refresh(1);
            m_stream->put(c);
            stream() << callStack;
            exit(1);
        }
    }
    else if (m_beenReset) {
        stream() << prefixString();
        m_beenReset = false;
    }

    m_stream->put(c);

    return 0;
}
