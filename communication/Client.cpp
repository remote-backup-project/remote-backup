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
    sendDirectory();
}

void Client::run(){
    ioContext.run();
}

void Client::sendHashFile(const std::string& filePath)
{
    LOG.debug("Client::sendHashFile - filePath = " + filePath);
    try{
        std::pair<std::string, int> md5FileTuple = StringUtils::md5FromFile(filePath);
        FileChunk fileChunk(md5FileTuple.first, filePath, StringUtils::getStringDifference(filePath, fileConfig.getInputDirPath()));
        sendRequest(this, Services::CHECKSUM_FILE, fileChunk.to_string(), sendContentFile);
    }
    catch(std::exception& exception){
        LOG.error("Client::sendHashFile - " + std::string(exception.what()));
    }
}

void Client::createRemoteDirectory(const std::string& directoryPath){
    LOG.debug("Client::createRemoteDirectory - directoryPath = " + directoryPath);
    std::string relativePath = StringUtils::getStringDifference(directoryPath, fileConfig.getInputDirPath());
    FileChunk fileChunk(directoryPath, relativePath);
    sendRequest(this, Services::TRANSFER_DIRECTORY, fileChunk.to_string(), [](void*, std::string){});
}

void Client::sendDirectory(){
    LOG.debug("Client::sendDirectory - directory = " + fileConfig.getInputDirPath());
    fs::path path(fileConfig.getInputDirPath());
    for(auto &p : fs::recursive_directory_iterator(path)){
        if( !fs::is_directory(p.status()) ){
            sendHashFile(p.path().string());
        }else createRemoteDirectory(p.path().string());
    }
}

void Client::sendRequest(void *context, std::string uri, std::string body, void (* onResponse)(void*, std::string))
{
    LOG.info("Client::sendRequest - URI = < " + uri + " >");
    auto* client = reinterpret_cast<Client*>(context);
    Request request(std::move(uri), std::move(body));

    client->clientConnectionPtr.reset(new ClientConnection(client->ioContext));
    client->clientConnectionPtr->setRequest(request);
    client->clientConnectionPtr->setCallback(context, onResponse);
    client->clientConnectionPtr->start();
}

void Client::sendContentFile(void* client, std::string filePath)
{
    LOG.debug("Client::sendContentFile - filePath = " + filePath);
    try{
        std::ifstream ifs(filePath, std::ios::in | std::ios::binary);

        if(!ifs.is_open()){
            LOG.error("Client::sendContentFile - cannot open file <" + filePath + ">");
            return;
        }

        std::vector<char> data(Socket::CHUNK_SIZE + 1, 0);
        while (ifs)
        {
            ifs.read(data.data(), Socket::CHUNK_SIZE);
            std::streamsize s = ifs.gcount();
            data[s] = 0;
            FileChunk fileChunk(std::string(data.data(), s), filePath, StringUtils::getStringDifference(filePath, fileConfig.getInputDirPath()));
            sendRequest(client, Services::TRANSFER_FILE, fileChunk.to_string(), [](void*, std::string){});
        }

        ifs.close();
    }
    catch(std::exception& exception){
        LOG.error("Client::sendContentFile - " + std::string(exception.what()));
    }
}
