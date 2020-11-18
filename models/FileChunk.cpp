//
// Created by alessandro on 07/11/20.
//

#include "FileChunk.h"
#include "../converters/Serializer.h"
#include "../utils/StringUtils.h"
#include <iostream>

FileChunk::FileChunk() {}

/* costruttore usato per la request di deleteResource */
FileChunk::FileChunk(std::string relativePath):
    chunkNumber(-1),
    relativePath(std::move(relativePath))
{}

FileChunk::FileChunk(std::string path, std::string relativePath):
        chunkNumber(0),
        path(std::move(path)),
        relativePath(std::move(relativePath)),
        isFile(false)
{}

FileChunk::FileChunk(std::string content, std::string path, std::string relativePath):
    content(std::move(content)),
    path(std::move(path)),
    relativePath(std::move(relativePath)),
    isFile(true)
{
    std::vector<std::string> results = StringUtils::split(this->path, "/");
    if(!results.empty())
        fileName = results[results.size()-1];
}

FileChunk::FileChunk(long chunkNumber, std::string content, std::string path, std::string relativePath):
    chunkNumber(chunkNumber),
    content(std::move(content)),
    path(std::move(path)),
    relativePath(std::move(relativePath)),
    isFile(true)
{
    std::vector<std::string> results = StringUtils::split(this->path, "/");
    if(!results.empty())
        fileName = results[results.size()-1];
}


bool FileChunk::end(){
    return this->path.empty();
}

bool FileChunk::isDirectory(){
    return !isFile;
}

void FileChunk::writeAsString(boost::property_tree::ptree& pt){
    pt.put("content", StringUtils::encodeBase64(this->content));
    pt.put("path", this->path);
    pt.put("fileName", this->fileName);
    pt.put("relativePath", this->relativePath);
    pt.put("chunkNumber", this->chunkNumber);
    pt.put("isFile", this->isFile);
}

void FileChunk::readAsString(boost::property_tree::ptree& pt){
    StringUtils::decodeBase64(pt.get<std::string>("content"), this->content);
    this->path = pt.get<std::string>("path");
    this->fileName = pt.get<std::string>("fileName");
    this->relativePath = pt.get<std::string>("relativePath");
    this->chunkNumber = pt.get<long>("chunkNumber");
    this->isFile = pt.get<bool>("isFile");
}

std::string FileChunk::getContent(){ return content; }

std::string FileChunk::getPath(){ return path; }

std::string FileChunk::getFilename() { return fileName; }

std::string FileChunk::getRelativePath() { return relativePath; }

long FileChunk::getChunkNumber(){ return chunkNumber; }

void FileChunk::setContent(std::string content){ this->content = std::move(content); }

std::string FileChunk::to_string(){
    auto string = Serializer::serialize(*this);
    return std::string(string.begin(), string.end());
}


