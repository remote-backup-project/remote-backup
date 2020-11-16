//
// Created by alessandro on 16/11/20.
//

#include "FileWriter.h"
#include "../utils/Logger.h"
#include "../utils/Constants.h"
#include <filesystem>

namespace fs = std::filesystem;

void FileWriter::write(std::string path, FileChunk& fileChunk)
{
    {
        std::unique_lock<std::mutex> lock(writerMutex);
        if(mutexes[path] == nullptr)
            mutexes[path] = std::make_unique<std::mutex>();
    }

    std::unique_lock<std::mutex> lock(*mutexes[path]);

    std::fstream ofs = fs::exists(path) ?
                       std::fstream(path, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate):
                       std::fstream(path, std::ios::out | std::ios::binary | std::ios::ate);

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
    if(pos > ofs.tellp().operator long())
        ofs.seekp(pos);

    ofs.close();
}

FileWriter fileWriter;