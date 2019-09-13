#pragma once

namespace lava {
    enum class RenderCategory {
        Opaque,      //< Render the mesh as fully opaque.
        Translucent, //< Render the mesh alpha-blended.
        Depthless,   //< Render behind all other objects and centered at the camera point of view.
        Wireframe,   //< Render the mesh as wireframe only.
    };
}
