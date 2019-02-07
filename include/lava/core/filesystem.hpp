#pragma once

#if __has_include(<filesystem>)
    #include <filesystem>
#else
    #include <experimental/filesystem>

    namespace std {
        using namespace experimental;
    }
#endif

namespace lava::fs {
    using Path = std::filesystem::path;

    inline bool isDirectory(const Path& path) { return std::filesystem::is_directory(path); }
    inline Path canonical(const Path& path) { return std::filesystem::canonical(path); }
}
