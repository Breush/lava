#pragma once

#include <lava/chamber/file-watch-event.hpp>

#include <optional>
#include <string>

namespace lava::chamber {
    class FileWatcher {
    public:
        FileWatcher();
        ~FileWatcher();

        /// Watch a file or a directory.
        void watch(const std::string& path);

        /// Grab the next event if any.
        std::optional<FileWatchEvent> pollEvent();

    private:
        class Impl;
        Impl* m_impl = nullptr;
    };
}
