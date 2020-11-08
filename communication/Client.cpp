//
// Created by alessandro on 07/11/20.
//

#include "Client.h"
#include <ostream>
#include <string>
#include <filesystem>
#include <utility>
#include "../utils/Logger.h"
#include "../utils/Constants.h"
#include "../utils/StringUtils.h"
#include "../models/FileConfig.h"

using boost::asio::ip::tcp;
namespace fs = std::filesystem;
namespace asio = boost::asio;


Client::Client(): clientConnectionPtr()
{
    fileConfig.readClientFile();
    sendDirectory(fileConfig.getInputDirPath());
}

void Client::run(){
    ioContext.run();
}

void Client::sendFile(const std::string& basePath, const std::string& filePath)
{
    LOG.info("Client::sendFile - filePath = " + filePath);
    try{
        std::string v2;
        std::ifstream ifs;
        ifs.open(filePath, std::ios::in | std::ios::binary);

        if(!ifs.is_open()){
            LOG.error("Client::sendFile - cannot open file <" + filePath + ">");
            return;
        }

        std::vector<char> data(Socket::CHUNK_DIMENSION + 1,0);
        while (true)
        {
            ifs.read(data.data(), Socket::CHUNK_DIMENSION);
            std::streamsize s = ((ifs)? Socket::CHUNK_DIMENSION : ifs.gcount());
            data[s] = 0;
            FileChunk fileChunk(std::string(data.data(), s), filePath, StringUtils::getStringDifference(filePath, basePath));
            sendData(Services::TRANSFER_FILE, fileChunk.to_string());

            if(!ifs)
                break;
        }
        ifs.close();
    }
    catch(std::exception& exception){
        LOG.error("Client::sendFile - " + std::string(exception.what()));
    }
}

void Client::createRemoteDirectory(const std::string& basePath, const std::string& directoryPath){
    LOG.info("Client::createRemoteDirectory - directoryPath = " + directoryPath);
    std::string relativePath = StringUtils::getStringDifference(directoryPath, basePath);
    FileChunk fileChunk("", directoryPath, relativePath);
    sendData(Services::TRANSFER_DIRECTORY, fileChunk.to_string());
}

void Client::sendDirectory(const std::string& directory){
    LOG.info("Client::sendDirectory - directory = " + directory);
    fs::path path(directory);
    for(auto &p : fs::recursive_directory_iterator(path)){
        if( !fs::is_directory(p.status()) ){
            sendFile(directory, p.path().string());
        }else createRemoteDirectory(directory, p.path().string());
    }
}

void Client::sendData(std::string uri, std::string body)
{
    Request request(std::move(uri), std::move(body));
    clientConnectionPtr.reset(new ClientConnection(ioContext));
    clientConnectionPtr->setRequest(request);
    clientConnectionPtr->start();
}
