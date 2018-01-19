#pragma once

#include <experimental/filesystem>

namespace lava::fs {
    using Path = std::experimental::filesystem::path;

    inline bool isDirectory(const Path& path) { return std::experimental::filesystem::is_directory(path); }
    inline Path canonical(const Path& path) { return std::experimental::filesystem::canonical(path); }
}
