/**
 * Shows how spatialization effect affects sounds.
 */

#include <lava/flow.hpp>

using namespace lava;
using namespace std::chrono;

int main(void)
{
    flow::AudioEngine engine;
    engine.listenerPosition({0.f, -50.f, 0.f});

    auto soundData = engine.share<flow::SoundData>("./assets/sounds/fire-truck.wav");
    auto& sound = engine.make<flow::Sound>(soundData);
    sound.looping(true);
    sound.play();

    // This sound will run in circle next to the listener.
    const auto soundSpeed = 50.f * 1000.f / 3600.f;                  // 50 km/h
    const auto wantedTimePerLoop = (2.f * M_PI * 50.f) / soundSpeed; // 50 m is the radius of the circle
    const auto soundAngularSpeed = (2.f * M_PI) / wantedTimePerLoop;

    duration<float> elapsedTotalTime = 0s;
    auto currentTime = high_resolution_clock::now();
    while (true) {
        auto elapsedTime = high_resolution_clock::now() - currentTime;
        elapsedTotalTime += elapsedTime;
        currentTime += elapsedTime;

        // Make it go in circle of radius 50 meters.
        const auto omega = soundAngularSpeed * elapsedTotalTime.count();
        glm::vec3 soundPosition = {0.f, 0.f, 0.f};
        soundPosition.x = -std::cos(omega) * 50.f;
        soundPosition.y = -std::sin(omega) * 50.f;
        sound.position(soundPosition);

        engine.update();
    }

    return EXIT_SUCCESS;
}
