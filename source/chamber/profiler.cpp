#include <lava/chamber/profiler.hpp>

#if defined(LAVA_CHAMBER_PROFILER_ENABLED)

#include <lava/chamber/logger.hpp>

using namespace lava;

void chamber::startProfilingOverNetwork()
{
    profiler::startListen();
}

void chamber::stopProfilingOverNetwork()
{
    profiler::stopListen();
}

void chamber::startProfiling()
{
    profiler::setEnabled(true);
}

void chamber::stopProfiling(std::string dumpFile)
{
    // Generating a
    if (dumpFile.empty()) {

        time_t rawtime;
        time(&rawtime);

        auto timeinfo = localtime(&rawtime);

        char buffer[50];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H:%M:%S", timeinfo);

        dumpFile = std::string(buffer) + ".prof";
    }

    auto blocksCount = profiler::dumpBlocksToFile(dumpFile.c_str());
    logger.info("chamber.profiler") << "Saved " << blocksCount << " blocks to " << dumpFile.c_str() << std::endl;
}

#endif
