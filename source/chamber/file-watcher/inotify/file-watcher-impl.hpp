#pragma once

#include <lava/chamber/file-watcher.hpp>

#include <atomic>
#include <queue>
#include <thread>
#include <unordered_map>

namespace lava::chamber {
    /// Inotify implementation of the FileWatcher.
    class FileWatcher::Impl {
    public:
        Impl();
        ~Impl();

        // FileWatcher
        uint32_t watch(const fs::Path& path);
        std::optional<FileWatchEvent> popEvent();

    private:
        uint32_t m_mask;
        int m_fileDescriptor = -1;
        std::unordered_map<int, fs::Path> m_watchDescriptors;
        std::queue<FileWatchEvent> m_eventsQueue;

        // Thread
        std::atomic<bool> m_watching{true};
        std::thread m_watchThread;
    };
}
