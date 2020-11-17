//
// Created by gaetano on 15/11/20.
//

#ifndef REMOTE_BACKUP_FILEWATCHER_H
#define REMOTE_BACKUP_FILEWATCHER_H

#include <string>
#include <chrono>
#include <filesystem>
#include <unordered_map>
#include <functional>
#include "Constants.h"

class FileWatcher {
public:
    FileWatcher(std::string path_to_watch);
    void start(const std::function<void (std::string, FileWatcherStatus::FileStatus)> &action);
    void stop();
    ~FileWatcher();
private:
    std::string path_to_watch;
    std::chrono::duration<int, std::milli> delay;
    std::unordered_map<std::string, std::filesystem::file_time_type> paths_;

    bool running_ = true;
    // Check if "paths_" contains a given key
    // If your compiler supports C++20 use paths_.contains(key) instead of this function
    bool contains(const std::string &key);
};


#endif //REMOTE_BACKUP_FILEWATCHER_H
