//
// Created by alessandro on 16/11/20.
//

#ifndef REMOTE_BACKUP_FILEWRITER_H
#define REMOTE_BACKUP_FILEWRITER_H

#include <iostream>
#include <memory>
#include <utility>
#include <mutex>
#include "./FileChunk.h"


class FileWriter {
    std::mutex writerMutex;
    std::map<std::string, std::unique_ptr<std::mutex>> mutexes;
public:
    FileWriter(){
    }

    void write(std::string path, FileChunk& fileChunk);
};

extern FileWriter fileWriter;

#endif //REMOTE_BACKUP_FILEWRITER_H
