#include <lava/chamber/tracker.hpp>

using namespace lava;

chamber::Tracker chamber::tracker;

using namespace lava::chamber;

uint32_t& Tracker::counter(const std::string& category)
{
    return m_counters[category];
}
