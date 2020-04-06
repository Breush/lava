#pragma once

#include <lava/core/macros/pimpl.hpp>

$enum_class(lava::magma, RendererType,
            /// Unknown
            Unknown,

            /**
             * Basic straight-forward renderer.
             *
             * Has two passes: one for opaque meshes and one for translucent ones.
             * Meshes have to be marked accordingly to render correctly.
             *
             * Artifacts might appear if tranlucent meshes intersects.
             *
             * Recommended for VR or mobile applications.
             */
            Forward,

            /**
             * A deferred renderer with a deep linked list for translucent fragments.
             *
             * Very costly memory-wise, it has however a perfect rendering of
             * translucent meshes, as the fragments are sorted on the GPU.
             * It does not use the mesh flag about translucent or not.
             *
             * It allows powerful post-processes that can use the linked list information,
             * and thus have better rendering of translucent surfaces.
             *
             * Recommended for small scenes with intensive translucency usage.
             */
            DeepDeferred,

            /**
             * A forward renderer specialized for rendering 2D elements (named flats).
             *
             * It's a one-pass renderer, everything has to be ordered.
             */
            ForwardFlat,

            // @note Upcoming: deferred renderer.
);
