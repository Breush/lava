#include <lava/chamber/file-watcher.hpp>

#if defined(LAVA_CHAMBER_FILEWATCHER_INOTIFY)
#include "./file-watcher/inotify/file-watcher-impl.hpp"
#elif defined(LAVA_CHAMBER_FILEWATCHER_WIN32)
#include "./file-watcher/win32/file-watcher-impl.hpp"
#else
#error "[lava.chamber.file-watcher] Unsupported platform for file watching"
#endif

using namespace lava::chamber;

$pimpl_class(FileWatcher);

$pimpl_method(FileWatcher, uint32_t, watch, const fs::Path&, path);

std::optional<FileWatchEvent> FileWatcher::pollEvent()
{
    return m_impl->popEvent();
}
