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
#include "./converters/Deserializer.h"
#include "./converters/Serializer.h"
#include "utils/Constants.h"
#include "models/FileInfo.h"
#include "models/StringWrapper.h"
#include <fstream>

namespace fs = std::filesystem;
namespace asio = boost::asio;

class Socket {
    asio::io_service io_service;
    asio::ip::tcp::socket socket;
    Socket(const Socket&) = delete;
    Socket& operator = (const Socket&) = delete;
    Socket(asio::io_service& io_service) : socket(io_service) {
        std::cout<<"Socket created - 1" <<std::endl;
    }
    friend class ServerSocket;

public:
    Socket() : socket(io_service){
        std::cout<<"Socket created - 2" <<std::endl;
    }
    ~Socket() {
        socket.close();
    }
    void connect(std::string ip_address, int port) {
        socket.connect(asio::ip::tcp::endpoint(
                asio::ip::address::from_string(ip_address),
                port));
    }

    template <typename T>
    T receiveData()
    {
        ushort size = 0;
        socket.receive(asio::buffer(&size, sizeof(ushort)));
        size_t sizeRecv = 0;
        std::vector<char> buff(size);
        do{
            sizeRecv += socket.receive(asio::buffer(reinterpret_cast<char*>(&buff[sizeRecv]), (size - sizeRecv) * sizeof(char)));
        }while(sizeRecv < size);
        return Deserializer::deserialize<T>(buff);
    }

    template <typename T>
    void sendData(T obj)
    {
        auto buff = Serializer::serialize<T>(obj);
        ushort size = buff.size();
        socket.send(asio::buffer(&size, sizeof(ushort)));
        size_t sizeSent = 0;
        do{
            sizeSent += socket.send(asio::buffer(reinterpret_cast<char*>(&buff[sizeSent]), (size - sizeSent)*sizeof(char)));
        }while(sizeSent < size);
    }

    void sendFile(const std::string& basePath, const std::string& filePath)
    {
        std::string v2;
        std::ifstream ifs;
        ifs.open(filePath, std::ios::in | std::ios::binary);

        if(!ifs.is_open())
            throw std::exception(); // TODO da rivedere se lanciare eccezione, altro o eccezione custom

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

    void finishSending(){
        sendData(FileInfo());
    }

    void createRemoteDirectory(const std::string& basePath, const std::string& directoryPath, asio::ip::tcp::socket& socket){
        std::string relativePath = StringUtils::getStringDifference(directoryPath, basePath);
        FileInfo fileInfo("", directoryPath, relativePath);
        sendData(fileInfo);
    }

    void sendDirectory(const std::string& directory){
        fs::path path(directory);
        for(auto &p : fs::recursive_directory_iterator(path)){
            if( !fs::is_directory(p.status()) ){
                sendFile(directory, p.path().string());
            }else createRemoteDirectory(directory, p.path().string(), socket);
        }
        finishSending();
    }

    void doLogin(){
        std::fstream clientFile;
        clientFile.open("/home/gaetano/CLionProjects/remote-backup/clientCredentials.txt");
        if(!clientFile.is_open()) std::cout<<"error opening client credentials file"<<std::endl;
        else {
            std::string temp;
            std::getline(clientFile, temp);
            std::string userName(temp);
            std::cout << "user: " << temp << std::endl;

            std::getline(clientFile, temp);
            std::string inputPath(temp);
            std::cout << "path: " << temp << std::endl;

            StringWrapper sentItem(userName + "\n" + inputPath);
            sendData(sentItem);
            sendDirectory(inputPath);
        }
        clientFile.close();
    }
};


#endif //REMOTE_BACKUP_SOCKET_H
