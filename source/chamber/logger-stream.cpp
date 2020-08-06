#include <lava/chamber/logger-stream.hpp>

#include <lava/chamber/string-tools.hpp>

using namespace lava::chamber;

LoggerStream::LoggerStream(std::ostream& stream, const std::string& resetString)
    : std::ostream(this)
    , m_stream(&stream)
    , m_resetString(resetString)
{
    auto infoLogLevelEnv = getenv("LAVA_INFO_LOG_LEVEL");
    m_infoLogLevel = (infoLogLevelEnv) ? std::atoi(infoLogLevelEnv) : -1u;
}

LoggerStream& LoggerStream::operator[](uint8_t i)
{
    static const std::string tab = "    ";
    if (i == 1) (*this) << tab * i;
    return *this;
}

LoggerStream& LoggerStream::tab(int8_t i)
{
    m_tabs += i;
    return *this;
}

std::string LoggerStream::tabsString()
{
    static const std::string tab("| ");
    return tab * m_tabs;
}

int LoggerStream::overflow(int c)
{
    if (m_kind == LoggerKind::Info && m_tabs >= m_infoLogLevel) {
        return 0;
    }

    if (c == '\n') {
        stream() << resetString();
        stream().flush();
        m_beenReset = true;

        if (m_autoExit) {
            stream() << std::endl;

            CallStack callStack;
            callStack.refresh(4);
            m_stream->put(c);
            stream() << callStack;
            exit(1);
        }
    }
    else if (m_beenReset) {
        stream() << prefixString() << tabsString();
        m_beenReset = false;
    }

    m_stream->put(c);

    return 0;
}
