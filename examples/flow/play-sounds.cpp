/**
 * Shows how to play multiple sounds at once.
 */

#include <lava/flow.hpp>

#include <chrono>

using namespace lava;
using namespace std::chrono;
using namespace std::chrono_literals;

int main(void)
{
    flow::AudioEngine engine;

    // Load the sound data only once, multiple sounds may use it at the same time.
    auto thunderSoundData = engine.share<flow::SoundData>("./assets/sounds/thunder.wav");

    auto currentTime = high_resolution_clock::now() - 1s;
    while (true) {
        // Start a new sound each second.
        auto elapsedTime = high_resolution_clock::now() - currentTime;
        if (elapsedTime > 1s) {
            currentTime += elapsedTime;

            // Generate the new sound, using preloaded data.
            auto& thunderSound = engine.make<flow::Sound>(thunderSoundData);
            thunderSound.playOnce();
        }

        engine.update(0.f);
    }

    return EXIT_SUCCESS;
}
