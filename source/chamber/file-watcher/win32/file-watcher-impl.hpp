#pragma once

#include <lava/chamber/file-watcher.hpp>

#include <atomic>
#include <experimental/filesystem>
#include <queue>
#include <thread>
#include <windows.h>

namespace lava::chamber {
    struct WatchHandleInfo {
        std::experimental::filesystem::path path;
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
        void watch(const std::string& path);
        std::optional<FileWatchEvent> popEvent();

    private:
        std::vector<WatchHandleInfo> m_watchHandlesInfos;
        std::queue<FileWatchEvent> m_eventsQueue;

        // Thread
        std::atomic<bool> m_watching{true};
        std::thread m_watchThread;
    };
}
