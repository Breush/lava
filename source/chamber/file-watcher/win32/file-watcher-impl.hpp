#pragma once

#include <lava/chamber/file-watcher.hpp>

#include <atomic>
#include <experimental/filesystem>
#include <queue>
#include <thread>
#include <windows.h>

namespace lava::chamber {
    struct WatchHandleInfo {
        fs::Path path;
        uint32_t watchId;
        bool directory = false;
        HANDLE handle;
        OVERLAPPED overlapped;
    };

    /// Win32 implementation of the FileWatcher.
    class FileWatcher::Impl {
    public:
        Impl();
        ~Impl();

        // FileWatcher
        uint32_t watch(const fs::Path& path);
        std::optional<FileWatchEvent> popEvent();

    private:
        std::vector<WatchHandleInfo> m_watchHandlesInfos;
        std::queue<FileWatchEvent> m_eventsQueue;

        // Thread
        std::atomic<bool> m_watching{true};
        std::thread m_watchThread;
    };
}
