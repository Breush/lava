#pragma once

#include <iostream>

#include <lava/chamber/call-stack.hpp>
#include <lava/core/macros.hpp>

namespace lava::chamber {
    enum class LoggerKind {
        Unknown,
        Info,
    };

    /**
     * Stream managing colors and reset upon line end.
     */
    class LoggerStream : public std::ostream, std::streambuf {
    public:
        /**
         * The reset string will be applied before each std::endl.
         */
        LoggerStream(std::ostream& stream, const std::string& resetString);

        /**
         * The targetted stream.
         */
        std::ostream& stream() { return *m_stream; }

        /**
         * Add a one-time spacing.
         */
        LoggerStream& operator[](uint8_t spacingLength);

        /**
         * Add a spacing to the prefix.
         * A negative one, will remove previous ones.
         */
        LoggerStream& tab(int8_t spacingLength);

    protected:
        int overflow(int c) override;

        std::string tabsString();

    private:
        std::ostream* m_stream = nullptr;

        $property(std::string, resetString);
        $property(std::string, prefixString);
        $property(bool, autoExit, = false);
        $property(LoggerKind, kind, = LoggerKind::Unknown);

        bool m_beenReset = true;
        uint8_t m_tabs = 0u;
        uint32_t m_infoLogLevel = 0u;
    };
}
