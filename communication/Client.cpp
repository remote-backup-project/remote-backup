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

using boost::asio::ip::tcp;
namespace fs = std::filesystem;
namespace asio = boost::asio;


Client::Client(std::string  server, std::string port):
        server(std::move(server)),
        port(std::move(port)),
        clientConnectionPtr()
{
    sendDirectory("/home/alessandro/CLionProjects/remote-backup/inputDirectory");
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

        std::vector<char> data(Constants::Pipe::MAX_BYTE + 1,0);
        while (true)
        {
            ifs.read(data.data(), Constants::Pipe::MAX_BYTE);
            std::streamsize s = ((ifs)? Constants::Pipe::MAX_BYTE : ifs.gcount());
            data[s] = 0;
            FileChunk fileChunk(std::string(data.data(), s), filePath, StringUtils::getStringDifference(filePath, basePath));
            sendData("/transfer/file", fileChunk.to_string());

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
    sendData("/transfer/directory", fileChunk.to_string());
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
    clientConnectionPtr.reset(new ClientConnection(ioContext, server, port));
    clientConnectionPtr->setRequest(request);
    clientConnectionPtr->start();
}
