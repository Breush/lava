#pragma once

namespace lava::chamber {
#define PROFILER_COLOR_UPDATE 0xFFAEC6CF     // Entity update
#define PROFILER_COLOR_RENDER 0xFFFFB347     // Command buffer creation
#define PROFILER_COLOR_DRAW 0xFFFF6961       // GPU drawing
#define PROFILER_COLOR_ALLOCATION 0xFF77DD77 // Memory allocation
#define PROFILER_COLOR_REGISTER 0xFFFDFD96   // Registering objects
#define PROFILER_COLOR_INIT 0xFFB19CD9       // Initializing/one-time context
}

// @note Currently no intrusive profiling tool is used.
// Hence, all of these are dummy.
// They might get removed.

namespace lava::chamber {
// #define PROFILE_ENABLED
#define PROFILE_FUNCTION(...) /* empty */
#define PROFILE_BLOCK(...)    /* empty */

    inline void startProfiling() {}
    inline void stopProfiling(std::string /* dumpFile */ = "") {}

    inline void profilerThreadName(const char* /* threadName */) {}
}
