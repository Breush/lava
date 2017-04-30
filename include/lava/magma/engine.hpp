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

        inline priv::EngineImpl& impl() { return *m_impl; }

    private:
        priv::EngineImpl* m_impl = nullptr;
    };
}
