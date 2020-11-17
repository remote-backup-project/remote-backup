//
// Created by gaetano on 15/11/20.
//

#include "FileWatcher.h"
#include <filesystem>
#include <string>
#include <functional>
#include <thread>
#include "Constants.h"
#include "Logger.h"

namespace fs = std::filesystem;

FileWatcher::FileWatcher(std::string path_to_watch) : path_to_watch(path_to_watch) {
    delay = std::chrono::milliseconds(5000); //controllo ogni 5sec se ci sono modifiche nella directory

    //memorizzo i path di file e cartelle così da sapere cosa viene modificato/creato/cancellato
    for(auto &file : std::filesystem::recursive_directory_iterator(path_to_watch)) {
        paths_[file.path().string()] = std::filesystem::last_write_time(file);
    }
}

void FileWatcher::start(const std::function<void (std::string, FileWatcherStatus::FileStatus)> &action) {
    //TODO valutare se differenziare RENAME da DELETE+CREATE
    running_ = true;
    while(running_) {
        // Wait for "delay" milliseconds
        std::this_thread::sleep_for(delay);
        auto it = paths_.begin();

        //Check if a file was erased
        while (it != paths_.end()) {
            if (!std::filesystem::exists(it->first)) {
                action(it->first, FileWatcherStatus::FileStatus::ERASED);
                it = paths_.erase(it);
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
                    paths_[file.path().string()] = current_file_last_write_time;
                    action(file.path().string(), FileWatcherStatus::FileStatus::CREATED);
                }
                // File modification
                else if(paths_[file.path().string()] != current_file_last_write_time) {
                    paths_[file.path().string()] = current_file_last_write_time;
                    action(file.path().string(), FileWatcherStatus::FileStatus::MODIFIED);
                }
            }
    }
}

void FileWatcher::stop() {
    running_ = false;
}

FileWatcher::~FileWatcher(){
    stop();
}

bool FileWatcher::contains(const std::string &key) {
    auto el = paths_.find(key);
    return el != paths_.end();
}