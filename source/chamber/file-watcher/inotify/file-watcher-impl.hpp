#pragma once

#include <lava/chamber/file-watcher.hpp>

#include <queue>
#include <thread>
#include <unordered_map>

namespace lava::chamber {
    /// Linux implementation of the FileWatcher.
    class FileWatcher::Impl {
    public:
        Impl();
        ~Impl();

        // FileWatcher
        void watch(const std::string& path);
        std::optional<FileWatchEvent> popEvent();

    private:
        uint32_t m_mask;
        int m_fileDescriptor = -1;
        std::unordered_map<int, std::string> m_watchDescriptors;
        std::queue<FileWatchEvent> m_eventsQueue;
        std::thread m_watchThread;
    };
}
