//
// Created by gaetano on 20/10/20.
//

#ifndef REMOTE_BACKUP_SOCKET_H
#define REMOTE_BACKUP_SOCKET_H

#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <boost/asio.hpp>
#include <filesystem>
#include <fstream>
#include "../converters/Deserializer.h"
#include "../converters/Serializer.h"
#include "../utils/Constants.h"
#include "../models/FileChunk.h"
#include "../models/StringWrapper.h"
#include "../exceptions/FileException.h"
#include "ServerConnection.h"
#include <boost/thread/thread.hpp>
#include "../models/Command.h"
#include "../models/FileConfig.h"

namespace fs = std::filesystem;
namespace asio = boost::asio;

//TODO da levare
class Socket {
    asio::io_service io_service;
    RequestHandler requestHandler;
    ConnectionPtr connectionPtr;
    Socket(const Socket&) = delete;
    Socket& operator = (const Socket&) = delete;
    Socket(asio::io_service& io_service) : socket(io_service),requestHandler(".") {
        LOG.info("Socket - socket created");
    }
    friend class ServerSocket;

public:
    asio::ip::tcp::socket socket;
    Socket() : socket(io_service), requestHandler("."){}
    ~Socket() {
        socket.close();
    }

    void close(){
        socket.close();
    }

    void connect(std::string hostname, int port) {
        LOG.info("Socket.connect - connecting to hostname = " + hostname + ", port = " + std::to_string(port));
        asio::ip::tcp::resolver resolver(io_service);
        while(true){
            asio::ip::tcp::endpoint endpoint = *resolver.resolve(asio::ip::tcp::resolver::query(hostname, std::to_string(port)));
            LOG.info("Socket.connect - hostname = " + hostname + " resolved into ip = " + endpoint.address().to_string());
            try{
                socket.connect(endpoint);
                connectionPtr.reset(new Connection(io_service, requestHandler));
                return;
            }
            catch(std::exception& exception){
                LOG.warning("Socket.connect - " + std::string(exception.what()) + ", retrying in 5 seconds");
                boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
            }
        }
    }

    template <typename T>
    T receiveData()
    {
        try{
            ushort size = 0;
            socket.receive(asio::buffer(&size, sizeof(ushort)));
            size_t sizeRecv = 0;
            std::vector<char> buff(size);
            do{
                sizeRecv += socket.receive(asio::buffer(reinterpret_cast<char*>(&buff[sizeRecv]), (size - sizeRecv) * sizeof(char)));
            }while(sizeRecv < size);
            return Deserializer::deserialize<T>(buff);
        }
        catch(std::exception& exception){
            LOG.error(exception.what());
        }
        exit(-1);
    }

    template <typename T>
    void sendData(T obj)
    {
        try{
            auto buff = Serializer::serialize<T>(obj);
            ushort size = buff.size();
            socket.send(asio::buffer(&size, sizeof(ushort)));
            size_t sizeSent = 0;
            do{
                sizeSent += socket.send(asio::buffer(reinterpret_cast<char*>(&buff[sizeSent]), (size - sizeSent)*sizeof(char)));
            }while(sizeSent < size);
        }
        catch(std::exception& exception){
            LOG.error(exception.what());
        }
    }

    void sendFile(const std::string& basePath, const std::string& filePath)
    {
        LOG.info("Socket.sendFile - filePath = " + filePath);
        try{
            std::string v2;
            std::ifstream ifs;
            ifs.open(filePath, std::ios::in | std::ios::binary);

            if(!ifs.is_open()){
                throw FileException("Cannot open file");
            }

            std::vector<char> data(Constants::Pipe::MAX_BYTE + 1,0);
            while (true)
            {
                ifs.read(data.data(), Constants::Pipe::MAX_BYTE);
                std::streamsize s = ((ifs)? Constants::Pipe::MAX_BYTE : ifs.gcount());
                data[s] = 0;
                FileChunk fileChunk(std::string(data.data(), s), filePath, StringUtils::getStringDifference(filePath, basePath));
                sendData(fileChunk);

                if(!ifs)
                    break;
            }
            ifs.close();
        }
        catch (FileException& exception) {
            LOG.error(exception.getMessage());
        }
        catch(std::exception& exception){
            LOG.error(exception.what());
        }
    }

    void finishSending(){
        LOG.info("Socket.finishSending");
        sendData(FileChunk());
    }

    void createRemoteDirectory(const std::string& basePath, const std::string& directoryPath){
        LOG.info("Socket.createRemoteDirectory - directoryPath = " + directoryPath);
        std::string relativePath = StringUtils::getStringDifference(directoryPath, basePath);
        FileChunk fileChunk("", directoryPath, relativePath);
        sendData(fileChunk);
    }

    void sendDirectory(const std::string& directory){
        LOG.info("Socket.sendDirectory - directory = " + directory);
        fs::path path(directory);
        for(auto &p : fs::recursive_directory_iterator(path)){
            if( !fs::is_directory(p.status()) ){
                sendFile(directory, p.path().string());
            }else this->createRemoteDirectory(directory, p.path().string());
        }
        finishSending();
    }

    void doLogin(){
        LOG.info("Socket.doLogin");
        try{
            Command sentItem(100, fileConfig.getUsername() + "\n" + fileConfig.getInputDirPath());
            sendData(sentItem);
            sendDirectory(fileConfig.getInputDirPath());
        }
        catch(std::exception& exception){
            LOG.error(exception.what());
        }
    }
};


#endif //REMOTE_BACKUP_SOCKET_H
