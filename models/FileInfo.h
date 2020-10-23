//
// Created by alessandro on 22/10/20.
//

#ifndef REMOTE_BACKUP_FILEINFO_H
#define REMOTE_BACKUP_FILEINFO_H

#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include "../utils/StringUtils.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

class FileInfo{
    std::string content;
    std::string path;
    std::string fileName;
    std::string relativePath;

public:
    FileInfo(){};
    FileInfo(std::string content, std::string path):
        content(std::move(content)),
        path(std::move(path))
    {
        std::vector<std::string> results = StringUtils::split(this->path, "/");
        if(!results.empty())
            fileName = results[results.size()-1];
    };
    FileInfo(std::string content, std::string path, std::string relativePath):
            content(std::move(content)),
            path(std::move(path)),
            relativePath(std::move(relativePath))
    {
        std::vector<std::string> results = StringUtils::split(this->path, "/");
        if(!results.empty())
            fileName = results[results.size()-1];
    };

    std::string getContent(){ return content; }
    std::string getPath(){ return path; }
    std::string getFilename() { return fileName; }
    std::string getRelativePath() { return relativePath; }

    bool isValid(){
        return !this->path.empty();
    }

    bool isDirectory(){
        return !this->path.empty() && this->content.empty();
    }

    void writeAsString(boost::property_tree::ptree& pt){
        pt.put("content", StringUtils::encodeBase64(this->content));
        pt.put("path", this->path);
        pt.put("fileName", this->fileName);
        pt.put("relativePath", this->relativePath);
    }

    void readAsString(boost::property_tree::ptree& pt){
        StringUtils::decodeBase64(pt.get<std::string>("content"), this->content);
        this->path = pt.get<std::string>("path");
        this->fileName = pt.get<std::string>("fileName");
        this->relativePath = pt.get<std::string>("relativePath");
    }

};

#endif //REMOTE_BACKUP_FILEINFO_H
