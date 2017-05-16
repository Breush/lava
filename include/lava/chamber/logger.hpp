#pragma once

#include <iostream>
#include <string>

#include <lava/chamber/logger-stream.hpp>

namespace lava {
    /**
     * Logger with different messages levels.
     */
    class Logger {
    public:
        Logger();

        /**
         * Log an output with information level.
         */
        LoggerStream& info(const std::string& category);

        /**
         * Log an output with warning level.
         */
        LoggerStream& warning(const std::string& category);

        /**
         * Log an output with highest severity level.
         */
        LoggerStream& error(const std::string& category);

    private:
        LoggerStream m_stream;
    };

    /**
     * Default logger.
     */
    extern Logger logger;
}
