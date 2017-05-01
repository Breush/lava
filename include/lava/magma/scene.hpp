#pragma once

namespace lava {
    class Engine;
}

namespace lava {
    /**
     * A Scene controls all the 3D elements to be drawn.
     *
     * It can be binded to a lava::Window.
     */
    class Scene {
    public:
        Scene(Engine& engine);
        ~Scene();

        /**
         * Render the whole scene.
         */
        void render();

    private:
        Engine& m_engine;
    };
}
