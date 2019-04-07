#pragma once

namespace lava::magma {
    /**
     * Helper to get whether VR can be used at runtime.
     *
     * It performs a quick check upon material availability.
     * But can return true even if the user configuration is wrong.
     *
     * Once a RenderEngine is created, one should check vrAvailable()
     * on it to see if it can really be used.
     */
    bool vrAvailable();
}
