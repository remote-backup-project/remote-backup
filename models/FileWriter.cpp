#include "FileWriter.h"
#include "../utils/Logger.h"
#include "../utils/Constants.h"
#include "../utils/StringUtils.h"
#include <filesystem>
#include <boost/algorithm/string.hpp>

namespace fs = std::filesystem;

void FileWriter::write(std::string basePath, FileChunk& fileChunk)
{
    std::string cachePath = basePath + "/.cache" + fileChunk.getRelativePath();
    {
        std::unique_lock<std::mutex> lock(writerMutex);
        if(mutexes[cachePath] == nullptr)
            mutexes[cachePath] = std::make_unique<std::mutex>();
    }

    std::unique_lock<std::mutex> lock(*mutexes[cachePath]);

    if(boost::equals(StringUtils::md5FromFile(basePath + fileChunk.getRelativePath()), fileChunk.getFileHash())){
        return;
    }

    std::fstream ofs = fs::exists(cachePath) ?
                       std::fstream(cachePath, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate):
                       std::fstream(cachePath, std::ios::out | std::ios::binary | std::ios::ate);

    if(!ofs.is_open())
        LOG.error("FileWriter::write - Cannot open/create received file on server");

    long pos = ofs.tellp().operator long();
    long initSize = pos < (fileChunk.getChunkNumber() - 1) * Socket::CHUNK_SIZE ?
                    (fileChunk.getChunkNumber() - 1) * Socket::CHUNK_SIZE - pos : 0;
    if(initSize > 0)
    {
        std::vector<char> contentInit(initSize, ' ');
        ofs.write(contentInit.data(), initSize*sizeof(char));
    }

    ofs.seekp((fileChunk.getChunkNumber() - 1) * Socket::CHUNK_SIZE);
    ofs.write(fileChunk.getContent().data(),fileChunk.getContent().size()*sizeof(char));
    if(pos > ofs.tellp().operator long()){
        ofs.seekp(pos);
    }


    ofs.close();

    if( (fs::file_size(fs::path(cachePath)) == fileChunk.getFileSize()) &&
            (boost::equals(StringUtils::md5FromFile(cachePath), fileChunk.getFileHash() ))) {
        fs::rename(fs::path(basePath + "/.cache" + fileChunk.getRelativePath()), // from folder
          fs::path(basePath + fileChunk.getRelativePath())); // to folder
    }
}

FileWriter fileWriter;