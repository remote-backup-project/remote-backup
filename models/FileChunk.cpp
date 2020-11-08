//
// Created by alessandro on 07/11/20.
//

#include "FileChunk.h"
#include "../converters/Serializer.h"
#include "../utils/StringUtils.h"
#include <iostream>

std::map<std::string, int> FileChunk::chunkNumberMap = {};

FileChunk::FileChunk() {}

FileChunk::FileChunk(std::string content, std::string path, std::string relativePath):
    content(std::move(content)),
    path(std::move(path)),
    relativePath(std::move(relativePath))
{
    std::vector<std::string> results = StringUtils::split(this->path, "/");
    if(!results.empty())
        fileName = results[results.size()-1];

    chunkNumber = ++chunkNumberMap[this->relativePath];
}

bool FileChunk::end(){
    return this->path.empty();
}

bool FileChunk::isDirectory(){
    return !this->path.empty() && this->content.empty();
}

void FileChunk::writeAsString(boost::property_tree::ptree& pt){
    pt.put("content", StringUtils::encodeBase64(this->content));
    pt.put("path", this->path);
    pt.put("fileName", this->fileName);
    pt.put("relativePath", this->relativePath);
    pt.put("chunkNumber", this->chunkNumber);
}

void FileChunk::readAsString(boost::property_tree::ptree& pt){
    StringUtils::decodeBase64(pt.get<std::string>("content"), this->content);
    this->path = pt.get<std::string>("path");
    this->fileName = pt.get<std::string>("fileName");
    this->relativePath = pt.get<std::string>("relativePath");
    this->chunkNumber = pt.get<int>("chunkNumber");
}

std::string FileChunk::getContent(){ return content; }

std::string FileChunk::getPath(){ return path; }

std::string FileChunk::getFilename() { return fileName; }

std::string FileChunk::getRelativePath() { return relativePath; }

int FileChunk::getChunkNumber(){ return chunkNumber; }

std::string FileChunk::to_string(){
    auto string = Serializer::serialize(*this);
    return std::string(string.begin(), string.end());
}

