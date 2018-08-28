#pragma once

#include <string>
#include <unordered_map>

namespace lava::chamber {
    /**
     * Tracking multiple data.
     */
    class Tracker {
    public:
        /**
         * Access the specified counter.
         */
        uint32_t& counter(const std::string& category);

    private:
        std::unordered_map<std::string, uint32_t> m_counters;
    };

    /**
     * Default tracker.
     */
    extern Tracker tracker;
}
