#pragma once

#include <lava/chamber/file-watch-event.hpp>

#include <optional>
#include <string>

namespace lava::chamber {
    class FileWatcher {
    public:
        FileWatcher();
        ~FileWatcher();

        /// Watch a file or a directory. Returns a watch Id.
        uint32_t watch(const fs::Path& path);

        /// Grab the next event if any.
        std::optional<FileWatchEvent> pollEvent();

    private:
        class Impl;
        Impl* m_impl = nullptr;
    };
}
