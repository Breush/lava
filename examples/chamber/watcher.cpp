/**
 * Shows the file watcher.
 */

#include <lava/chamber.hpp>

#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

using namespace lava;
using namespace std::chrono_literals;

int main(void)
{
    // Watch a directory
    chamber::FileWatcher watcher;
    watcher.watch("./data/tmp");

    // Modify a file
    std::ofstream file("./data/tmp/watcher.tmp");
    file << "Hello" << std::endl;
    file << "World" << std::endl;
    file.close();

    std::this_thread::sleep_for(500ms);

    // Get all events
    while (auto event = watcher.pollEvent()) {
        std::cout << "FileWatcher: " << event->path;
        switch (event->type) {
        case chamber::FileWatchEvent::Type::Modified: std::cout << " modified." << std::endl; break;
        case chamber::FileWatchEvent::Type::Created: std::cout << " created." << std::endl; break;
        case chamber::FileWatchEvent::Type::Deleted: std::cout << " deleted." << std::endl; break;
        }
    }

    return EXIT_SUCCESS;
}
