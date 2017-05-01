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

    private:
        priv::EngineImpl* m_impl = nullptr;
    };
}
