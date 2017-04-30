#pragma once

#include <lava/crater/Window.hpp>

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
        Scene(Engine& engine, lava::Window& window);
        ~Scene();

        /**
         * Bind or rebind the scene to a lava::Window.
         */
        void bind(lava::Window& window);

        /**
         * Render the whole scene.
         */
        void render();

    private:
        Engine& m_engine;
        lava::Window* m_window = nullptr;
    };
}
