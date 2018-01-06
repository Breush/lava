#include "./file-watcher-impl.hpp"

#include <lava/chamber/logger.hpp>

using namespace lava;

namespace {
    static constexpr const auto fileNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                                                   // FILE_NOTIFY_CHANGE_ATTRIBUTES | // Ignored (nobody cares)
                                                   // FILE_NOTIFY_CHANGE_SIZE | // Ignored (LAST_WRITE does a better job)
                                                   FILE_NOTIFY_CHANGE_LAST_WRITE |
                                                   // FILE_NOTIFY_CHANGE_LAST_ACCESS | // Ignored (read)
                                                   // FILE_NOTIFY_CHANGE_CREATION | // Ignored (creation time)
                                                   // FILE_NOTIFY_CHANGE_SECURITY // Ignored
                                                   0x0;

    void waitEvents(std::vector<chamber::WatchHandleInfo>* handlesInfos, const std::atomic<bool>* watching,
                    std::queue<chamber::FileWatchEvent>* eventsQueue)
    {
        // @note All these bytes are for filenames and such
        FILE_NOTIFY_INFORMATION fileNotifyInformations[1024];
        std::vector<HANDLE> overlappedEvents;
        std::vector<uint32_t> handleInfoIndexes;

        while (*watching) {
            if (!handlesInfos->size()) continue;

            overlappedEvents.clear();
            for (auto& handleInfo : *handlesInfos) {
                ReadDirectoryChangesW(handleInfo.handle, &fileNotifyInformations, sizeof(fileNotifyInformations), true,
                                      fileNotifyFilter, nullptr, &handleInfo.overlapped, nullptr);

                overlappedEvents.emplace_back(handleInfo.overlapped.hEvent);
            }

            auto waitStatus = WaitForMultipleObjects(overlappedEvents.size(), overlappedEvents.data(), false, 200);
            switch (waitStatus) {
            case WAIT_ABANDONED: break;
            case WAIT_TIMEOUT: break;
            case WAIT_FAILED: break;
            default: {
                // If there is a next entry offset, there should have been more to do!
                if (fileNotifyInformations[0].NextEntryOffset) {
                    chamber::logger.warning("chamber.file-watcher") << "Might have lost some information..." << std::endl;
                }

                // @todo Shouldn't need wstring conversion as it should be std::filesystem::path,
                // using OS structure.

                // @note FileName is not null terminated
                auto fileName = fileNotifyInformations[0].FileName;
                std::wstring wPath(fileName, fileName + fileNotifyInformations[0].FileNameLength / sizeof(fileName[0]));
                std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wConverter;

                const auto& handleInfo = handlesInfos->at(waitStatus - WAIT_OBJECT_0);
                if (!handleInfo.directory) {
                    // If watching a file, ignore other file change in this directory
                    if (handleInfo.path.filename().c_str() != wPath) {
                        break;
                    }
                    else {
                        wPath = handleInfo.path.c_str();
                    }
                }
                else {
                    wPath = (handleInfo.path / wPath).c_str();
                }

                chamber::FileWatchEvent event;
                event.path = wConverter.to_bytes(wPath);

                switch (fileNotifyInformations[0].Action) {
                case FILE_ACTION_ADDED: event.type = chamber::FileWatchEvent::Type::Created; break;
                case FILE_ACTION_REMOVED: event.type = chamber::FileWatchEvent::Type::Deleted; break;
                case FILE_ACTION_MODIFIED: event.type = chamber::FileWatchEvent::Type::Modified; break;
                case FILE_ACTION_RENAMED_OLD_NAME: event.type = chamber::FileWatchEvent::Type::Deleted; break;
                case FILE_ACTION_RENAMED_NEW_NAME: event.type = chamber::FileWatchEvent::Type::Created; break;
                default: {
                    chamber::logger.warning("chamber.file-watcher")
                        << "File notify action is unknown: " << fileNotifyInformations[0].Action << "." << std::endl;
                    continue;
                }
                }

                eventsQueue->emplace(event);
                break;
            }
            }
        }
    }
}

using namespace lava::chamber;

FileWatcher::Impl::Impl()
    : m_watchThread(waitEvents, &m_watchHandlesInfos, &m_watching, &m_eventsQueue)
{
}

FileWatcher::Impl::~Impl()
{
    m_watching = false;
    m_watchThread.join();

    for (const auto& handleInfo : m_watchHandlesInfos) {
        FindCloseChangeNotification(handleInfo.handle);
    }
}

void FileWatcher::Impl::watch(const std::string& path)
{
    WatchHandleInfo handleInfo;

    // @todo Get a filesystem::path in the interface
    handleInfo.path = path;
    handleInfo.directory = std::experimental::filesystem::is_directory(handleInfo.path);

    // If watching a file, watch its directory...
    auto watchedPath = handleInfo.directory ? handleInfo.path : handleInfo.path.parent_path();
    handleInfo.handle = CreateFile(watchedPath.c_str(), GENERIC_READ | FILE_LIST_DIRECTORY,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
                                   FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);

    if (handleInfo.handle == INVALID_HANDLE_VALUE) {
        logger.warning("chamber.file-watcher") << "Unable to watch " << path << "." << std::endl;
        return;
    }

    handleInfo.overlapped.OffsetHigh = 0;
    handleInfo.overlapped.hEvent = CreateEvent(nullptr, true, false, nullptr);

    m_watchHandlesInfos.emplace_back(handleInfo);
}

std::optional<FileWatchEvent> FileWatcher::Impl::popEvent()
{
    if (m_eventsQueue.empty()) return std::nullopt;

    auto event = m_eventsQueue.front();
    m_eventsQueue.pop();
    return event;
}
