#pragma once

#include <iostream>
#include <string>

#include <lava/chamber/properties.hpp>

namespace lava {
    /**
     * Stream managing colors and reset upon line end.
     */
    class LoggerStream {
    public:
        using CoutType = decltype(std::cout);
        using EndlType = CoutType&(CoutType&);

        /**
         * The reset string will be applied before each std::endl.
         */
        LoggerStream(std::ostream& stream, const std::string& resetString);

        /**
         * Add a spacing.
         */
        LoggerStream& operator[](uint8_t i);

        /**
         * Streaming wrappers.
         */
        LoggerStream& operator<<(const std::string& string);
        LoggerStream& operator<<(int64_t number);
        LoggerStream& operator<<(uint64_t number);
        LoggerStream& operator<<(EndlType endl);

    private:
        std::ostream* m_stream = nullptr;

        $property(std::string, resetString);
        $property(bool, autoExit, = false);
    };
}
