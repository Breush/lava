#pragma once

#include <lava/flow/audio-source.hpp>

#include <glm/vec3.hpp>
#include <memory>

namespace lava::flow {
    class AudioEngine {
    public:
        AudioEngine();
        ~AudioEngine();

        /**
         * To be called to mix and play all currently active audio.
         * Should be called regularly, like a rendering function.
         */
        void update();

        /**
         * @name Makers
         * Make a new resource and add it to the engine.
         *
         * Arguments will be forwarded to the constructor.
         * Any resource that match an adder (see below) can be made.
         *
         * ```
         * auto& sound = engine.make<Sound>(); // Its lifetime is now managed by the engine.
         * ```
         */
        /// @{
        /// Make a new resource directly.
        template <class T, class... Arguments>
        T& make(Arguments&&... arguments);
        /// @}

        /**
         * @name Adders
         * Add a resource that has already been created.
         *
         * Its ownership goes to the engine.
         * For convenience, you usually want to use makers (see above).
         */
        /// @{
        void add(std::unique_ptr<AudioSource>&& source);
        /// @}

        /**
         * @name Sharers
         * Make a new resource and return its reference.
         * Its ownership doesn't go to the engine,
         * it will just live as long as needed.
         *
         * Arguments will be forwarded to the constructor.
         *
         * ```
         * auto soundData = engine.make<SoundData>("./sound.wav");
         * ```
         */
        /// @{
        /// Make a new resource directly.
        template <class T, class... Arguments>
        std::shared_ptr<T> share(Arguments&&... arguments);
        /// @}

        /**
         * @name Spatialization
         * Control over the listener of spatialized audio.
         */
        /// @{
        void listenerPosition(const glm::vec3& listenerPosition);
        /// @}

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}

#include <lava/flow/audio-engine.inl>
