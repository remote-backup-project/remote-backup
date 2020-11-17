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
#include "../utils/FileWatcher.h"

using boost::asio::ip::tcp;
namespace fs = std::filesystem;
namespace asio = boost::asio;


Client::Client(): clientConnectionPtr()
{
    fileConfig.readClientFile();
    FileWatcher fileWatcher(fileConfig.getInputDirPath());
    //TODO opzione 1
        //TODO se per server la cartella non esiste chiamare send Directory si InputDirPath
        //TODO se per server già esisteva, affidarsi a fileWatcher
    //TODO opzione 2
        //TODO reinviare tutta la cartella a prescindere alla prima connessione e poi affidarsi a fileWatcher
    fileWatcher.start([&] (std::string news_on_path, FileWatcherStatus::FileStatus status) -> void {
        // Process only regular files and directories, all other file types are ignored
        if(!fs::is_regular_file(fs::path(news_on_path)) && !fs::is_directory(fs::path(news_on_path))
            && status != FileWatcherStatus::FileStatus::ERASED) {
            return;
        }

        switch(status) {
            case FileWatcherStatus::FileStatus::CREATED: {
                if(fs::is_regular_file(fs::path(news_on_path))){
                    LOG.debug("CREATED FILE - " + news_on_path);
                    sendHashFile(news_on_path);
                }
                else if(fs::is_directory(fs::path(news_on_path))){
                    LOG.debug("CREATED DIRECTORY- " + news_on_path);
                    createRemoteDirectory(news_on_path);
                }
                break;
            }
            case FileWatcherStatus::FileStatus::MODIFIED: {
                if(fs::is_regular_file(fs::path(news_on_path))){
                    LOG.debug("MODIFIED FILE- " + news_on_path);
                    sendHashFile(news_on_path);
                }
                break;
            }
            case FileWatcherStatus::FileStatus::ERASED: {
                LOG.debug("DELETED - " + news_on_path);
                //TODO funzione per eliminare file/directory con path "news_on_path"
                break;
            }
            default: {
                LOG.error("Error! Unknown file status.");
            }
        }
    });

    sendDirectory(fileConfig.getInputDirPath());
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

void Client::sendDirectory(const std::string& directoryPath){
    LOG.debug("Client::sendDirectory - directory = " + fileConfig.getInputDirPath());
    fs::path path(directoryPath);
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
