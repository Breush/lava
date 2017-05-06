#pragma once

#include <lava/crater/Window.hpp>

namespace lava::priv {
    class EngineImpl;
}

namespace lava {
    /**
     * An Engine manages should have a unique instance as it initializes
     * the rendering pipeline.
     */
    class Engine {
    public:
        Engine(lava::Window& window);
        ~Engine();

        void draw();
        void mode(const lava::VideoMode& mode);

    private:
        priv::EngineImpl* m_impl = nullptr;
    };
}
