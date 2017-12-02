#include <lava/chamber/file-watcher.hpp>

#include <lava/chamber/macros.hpp>

// @todo Multi-platform
#if defined(LAVA_CHAMBER_FILEWATCHER_INOTIFY)
#include "./file-watcher/inotify/file-watcher-impl.hpp"
#endif

using namespace lava::chamber;

$pimpl_class(FileWatcher);

$pimpl_method(FileWatcher, void, watch, const std::string&, path);

std::optional<FileWatchEvent> FileWatcher::pollEvent()
{
    return m_impl->popEvent();
}
