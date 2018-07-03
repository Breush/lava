#pragma once

#include <lava/flow/sample-format.hpp>

#include <lava/chamber/logger.hpp>
#include <pulse/pulseaudio.h>

namespace lava::flow::helpers {
    inline pa_sample_format_t pulseSampleFormat(SampleFormat sampleFormat)
    {
        switch (sampleFormat) {
        case SampleFormat::Int16LE: return PA_SAMPLE_S16LE;
        case SampleFormat::Float32: return PA_SAMPLE_FLOAT32;
        default: break;
        }

        chamber::logger.error("flow.pulse.sample-helper") << "Unhandled sample format: " << sampleFormat << "." << std::endl;
        return static_cast<pa_sample_format>(0u);
    }
}
