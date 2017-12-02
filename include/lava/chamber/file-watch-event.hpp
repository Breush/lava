#pragma once

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
        std::string path;
    };
}
