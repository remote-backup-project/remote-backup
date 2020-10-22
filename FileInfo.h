//
// Created by alessandro on 22/10/20.
//

#ifndef REMOTE_BACKUP_FILEINFO_H
#define REMOTE_BACKUP_FILEINFO_H

#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include "utils/Utils.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>

class FileInfo{
    std::string content;
    std::string path;

public:
    FileInfo(){};
    FileInfo(std::string content, std::string path): content(std::move(content)), path(std::move(path)){};
    FileInfo(std::string path): path(std::move(path)){};

    std::string getContent(){ return content; }
    std::string getPath(){ return path; }
    std::string getFilename() {
        std::vector<std::string> results;
        boost::split(results, this->path, boost::is_any_of("/"));
        return results[results.size()-1];
    }

    bool isValid(){
        return !this->path.empty();
    }

    bool isDirectory(){
        return !this->path.empty() && this->content.empty();
    }

    void writeAsString(boost::property_tree::ptree& pt){
        pt.put("path", this->path);
        pt.put("content", Utils::encodeBase64(this->content));
    }

    void readAsString(boost::property_tree::ptree& pt){
        this->path = pt.get<std::string>("path");
        Utils::decodeBase64(pt.get<std::string>("content"), this->content);
    }

};

#endif //REMOTE_BACKUP_FILEINFO_H
