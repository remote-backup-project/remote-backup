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
#include "../models/FileInfo.h"
#include "../models/Command.h"
#include "../exceptions/FileException.h"
#include "../models/FileConfig.h"

namespace fs = std::filesystem;
namespace asio = boost::asio;

class Socket {
    asio::io_service io_service;
    asio::ip::tcp::socket socket;
    Socket(const Socket&) = delete;
    Socket& operator = (const Socket&) = delete;
    Socket(asio::io_service& io_service) : socket(io_service) {
        LOG.info("Socket - socket created");
    }
    friend class ServerSocket;

public:
    Socket() : socket(io_service){}
    ~Socket() {
        socket.close();
    }

    void close(){
        socket.close();
    }

    void connect(std::string ip_address, int port) {
        LOG.info("Socket.connect - connecting to ip = " + ip_address + ", port = " + std::to_string(port));
        asio::ip::tcp::resolver resolver(io_service);
        asio::ip::tcp::resolver::results_type results = resolver.resolve("AR-PC", std::to_string(port));
        for(asio::ip::tcp::endpoint const& endpoint : results)
        {
            std::cout << endpoint << "\n";
            try{
                socket.connect(endpoint);
            }
            catch(std::exception& exception){
                LOG.error(exception.what());
                exit(-1); //TODO considerare che connect non è bloccante e quindi va rifatta ogni x tempo
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
                FileInfo fileInfo(std::string(data.data(), s), filePath, StringUtils::getStringDifference(filePath, basePath));
                sendData(fileInfo);

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
        sendData(FileInfo());
    }

    void createRemoteDirectory(const std::string& basePath, const std::string& directoryPath){
        LOG.info("Socket.createRemoteDirectory - directoryPath = " + directoryPath);
        std::string relativePath = StringUtils::getStringDifference(directoryPath, basePath);
        FileInfo fileInfo("", directoryPath, relativePath);
        sendData(fileInfo);
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

    void doLogin(FileConfig fileConfig){
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
