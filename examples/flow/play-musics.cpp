/**
 * Shows how to play multiple musics at once.
 */

#include <lava/flow.hpp>

#include <chrono>
#include <vector>

using namespace lava;
using namespace std::chrono;
using namespace std::chrono_literals;

int main(void)
{
    flow::AudioEngine engine;

    // This loads the file data into memory (without decompressing it).
    auto buddyMusicData = engine.share<flow::FileMusicData>("./assets/musics/buddy.ogg");

    auto currentTime = high_resolution_clock::now() - 5s;
    while (true) {
        // Start a new music after 5 seconds.
        auto elapsedTime = high_resolution_clock::now() - currentTime;
        if (elapsedTime > 5s) {
            currentTime += elapsedTime;

            // Musics files are decompressed and streamed live.
            auto& music = engine.make<flow::Music>(buddyMusicData);
            music.play();
        }

        engine.update();
    }

    return EXIT_SUCCESS;
}
