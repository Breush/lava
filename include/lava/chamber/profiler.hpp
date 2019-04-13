#pragma once

namespace lava::chamber {
#define PROFILER_COLOR_UPDATE 0xFFAEC6CF     // Entity update
#define PROFILER_COLOR_RENDER 0xFFFFB347     // Command buffer creation
#define PROFILER_COLOR_DRAW 0xFFFF6961       // GPU drawing
#define PROFILER_COLOR_ALLOCATION 0xFF77DD77 // Memory allocation
#define PROFILER_COLOR_REGISTER 0xFFFDFD96   // Registering objects
#define PROFILER_COLOR_INIT 0xFFB19CD9       // Initializing/one-time context
}

#if defined(LAVA_CHAMBER_PROFILER_ENABLED)

#define BUILD_WITH_EASY_PROFILER
#define EASY_OPTION_LOG_ENABLED
#include <easy/profiler.h>

namespace lava::chamber {
#define PROFILE_ENABLED
#define PROFILE_FUNCTION(...) EASY_BLOCK(__PRETTY_FUNCTION__, ##__VA_ARGS__)

    void startProfilingOverNetwork();
    void stopProfilingOverNetwork();

    void startProfiling();
    void stopProfiling(std::string dumpFile = "");

    inline void profilerThreadName(const char* threadName) { EASY_THREAD(threadName); }
}

#else

namespace lava::chamber {
// #define PROFILE_ENABLED
#define PROFILE_FUNCTION(...) /* empty */

    inline void startProfilingOverNetwork() {}
    inline void stopProfilingOverNetwork() {}

    inline void startProfiling() {}
    inline void stopProfiling(std::string /* dumpFile */ = "") {}

    inline void profilerThreadName(const char* /* threadName */) {}
}

#endif
