#pragma once

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
        Engine();

    private:
        priv::EngineImpl* m_impl = nullptr;
    };
}
