#pragma once

#include <lava/flow/sample-format.hpp>

#include <cstdint>

namespace lava::flow {
    /**
     * Interface for music decoders.
     */
    class IMusicDecoder {
    public:
        virtual ~IMusicDecoder() = default;

        /// Sample rate (in Hz).
        virtual uint32_t rate() const = 0;

        /// Sample channels (1 for mono, 2 for stereo and more if needed).
        virtual uint8_t channels() const = 0;

        /// Sample format.
        virtual SampleFormat sampleFormat() const = 0;

        /**
         * Size of the currently loaded frame (in bytes).
         * If 0, this is the end of the stream.
         */
        virtual uint32_t frameSize() const = 0;

        /**
         * Data pointer to the currently loaded frame.
         * The real underlying type depends on the sample format.
         */
        virtual const uint8_t* frameData() const = 0;

        /**
         * Request loading of the next frame (in bytes).
         * It can be lower than requested at the end of the stream,
         * or due to alignment issues.
         */
        virtual void acquireNextFrame(uint32_t frameSize) = 0;

        /// Seek the stream to the start.
        virtual void seekStart() = 0;
    };
}
