#pragma once

#include <lava/crater/Window.hpp>

namespace lava {
    /**
     * An Engine manages should have a unique instance as it initializes
     * the rendering pipeline.
     */
    class Engine {
    public:
        Engine(lava::Window& window);
        ~Engine();

        class Impl;
        Impl& impl() const { return *m_impl; }

        void draw();
        void update();

        void mode(const lava::VideoMode& mode);

    private:
        Impl* m_impl = nullptr;
    };
}
