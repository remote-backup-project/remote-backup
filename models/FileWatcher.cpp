//
// Created by gaetano on 15/11/20.
//

#include "FileWatcher.h"
#include <filesystem>
#include <string>
#include <functional>
#include <thread>
#include "../utils/Logger.h"
#include "../models/FileConfig.h"

namespace fs = std::filesystem;

FileWatcher::FileWatcher(){}

void FileWatcher::start(boost::asio::io_context& ioContext, const std::function<void (std::string, FileWatcherStatus::FileStatus)> &action) {
    //TODO valutare se differenziare RENAME da DELETE+CREATE
    path_to_watch = fileConfig.getInputDirPath();
    LOG.trace("FileWatcher::start - inputDirPath = " + path_to_watch);
    running = true;

    while(running) {
        // Wait for 3000 milliseconds
        auto it = paths.begin();

        //Check if a file was erased
        while (it != paths.end()) {
            if (!std::filesystem::exists(it->first)) {
                action(it->first, FileWatcherStatus::FileStatus::ERASED);
                it = paths.erase(it);
            }
            else {
                it++;
            }
        }
        // Check if a file was created or modified
        for(auto &file : std::filesystem::recursive_directory_iterator(path_to_watch)) {
            auto current_file_last_write_time = std::filesystem::last_write_time(file);
            // File creation
            if(!contains(file.path().string())) {
                paths[file.path().string()] = current_file_last_write_time;
                action(file.path().string(), FileWatcherStatus::FileStatus::CREATED);
            }
            // File modification
            else if(paths[file.path().string()] != current_file_last_write_time) {
                paths[file.path().string()] = current_file_last_write_time;
                action(file.path().string(), FileWatcherStatus::FileStatus::MODIFIED);
            }
        }

        ioContext.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
}

void FileWatcher::stop() {
    running = false;
}

FileWatcher::~FileWatcher(){
    stop();
}

bool FileWatcher::contains(const std::string &key) {
    auto el = paths.find(key);
    return el != paths.end();
}