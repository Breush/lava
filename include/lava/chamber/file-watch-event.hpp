#pragma once

#include <lava/core/filesystem.hpp>
#include <string>

namespace lava::chamber {
    /**
     * A file or directory event.
     */
    struct FileWatchEvent {
        enum class Type {
            Modified,
            Created,
            Deleted,
        };

        Type type;
        fs::Path path;
        uint32_t watchId;
    };
}
