//
// Created by alessandro on 22/10/20.
//

#ifndef REMOTE_BACKUP_FILECHUNK_H
#define REMOTE_BACKUP_FILECHUNK_H

#include <string>
#include <map>
#include "Serializable.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

class FileChunk : public Serializable{
    static std::map<std::string, int> chunkNumberMap;
    int chunkNumber;
    std::string content;
    std::string path;
    std::string fileName;
    std::string relativePath;

public:
    FileChunk();
    FileChunk(std::string content, std::string path, std::string relativePath);

    std::string getContent();
    std::string getPath();
    std::string getFilename();
    std::string getRelativePath();
    int getChunkNumber();

    bool end();

    bool isDirectory();

    void writeAsString(boost::property_tree::ptree& pt) override;

    void readAsString(boost::property_tree::ptree& pt) override;

    std::string to_string() override;
};


#endif //REMOTE_BACKUP_FILECHUNK_H
