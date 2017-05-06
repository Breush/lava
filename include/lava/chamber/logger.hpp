#pragma once

#include <iostream>
#include <string>

namespace lava::logger {
    /**
     * Log an output with information level.
     */
    inline std::ostream& info(const std::string& category)
    {
        std::cout << "\e[1;39m[" << category << "] \e[1;32m";
        return std::cout;
    }

    /**
     * Log an output with warning level.
     */
    inline std::ostream& warning(const std::string& category)
    {
        std::cout << "\e[1;39m[" << category << "] \e[1;33m/!\\ ";
        return std::cout;
    }

    /**
     * Log an output with highest severity level.
     */
    inline std::ostream& error(const std::string& category)
    {
        std::cout << "\e[1;39m[" << category << "] \e[1;31m/!\\ /!\\ ";
        return std::cout;
    }

    /**
     * Add spacing.
     */
    constexpr const char* sub(int i)
    {
        if (i <= 0) return "";
        if (i == 1) return "    ";
        if (i == 2) return "        ";
        if (i == 3) return "            ";
        return "                ";
    }
}
