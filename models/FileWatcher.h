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
#include "../utils/Constants.h"
#include <boost/asio.hpp>

class FileWatcher {
public:
    FileWatcher();
    void start(boost::asio::io_context& ioContext, const std::function<void (std::string, FileWatcherStatus::FileStatus)> &action);
    void stop();
    ~FileWatcher();
private:
    std::string path_to_watch;
    std::unordered_map<std::string, std::filesystem::file_time_type> paths;

    bool running = true;
    // Check if "paths" contains a given key
    // If your compiler supports C++20 use paths.contains(key) instead of this function
    bool contains(const std::string &key);
};


#endif //REMOTE_BACKUP_FILEWATCHER_H
