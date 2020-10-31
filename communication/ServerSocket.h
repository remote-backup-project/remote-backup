//
// Created by gaetano on 20/10/20.
//

#ifndef REMOTE_BACKUP_SERVERSOCKET_H
#define REMOTE_BACKUP_SERVERSOCKET_H

#include <cstdint>
#include <stdexcept>
#include <cstring>
#include "Socket.h"
#include "../models/Command.h"
#include <algorithm>
#include "../utils/Logger.h"

class ServerSocket : public Socket{
    asio::ip::tcp::acceptor::endpoint_type end_type;
public:
    ServerSocket(){}
    ~ServerSocket(){
        socket.close();
    }

    void close(){
        socket.close();
    }

    bool is_open(){
        return socket.is_open();
    }

    void accept(int port) {
        LOG.info("ServerSocket.accept - port = " + std::to_string(port));
        try{
            asio::ip::tcp::acceptor acceptor_server(io_service,
                                                    asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
            LOG.info("ServerSocket.accept - Waiting for incoming connections on port = " + std::to_string(port));
            acceptor_server.accept(socket, end_type); // Waiting for connection
            std::string sClientIp = end_type.address().to_string();
//            unsigned short uiClientPort = end_type.port();
            LOG.info("ServerSocket.accept - " + sClientIp + " connected on port: " + std::to_string(port));
        }
        catch(std::exception& exception){
            LOG.error("ServerSocket.accept - " + std::string(exception.what()) + " - port = " + std::to_string(port));
        }
    }

    void receiveDirectory(const std::string& outputDir){
        LOG.info("ServerSocket.receiveDirectory - outputDir = " + outputDir);
        try{
            std::ofstream ofs3;
            while(true){
                auto fileInfo = receiveData<FileInfo>();

                // serve per bloccare la lettura da pipe/socket
                if(fileInfo.end()){
                    LOG.info("ServerSocket.receiveDirectory - finishReceiving");
                    return;
                }

                if(fileInfo.isDirectory()){
                    fs::path path(outputDir + fileInfo.getRelativePath());
                    LOG.info("ServerSocket.receiveDirectory - directory = " + path.string());

                    if(!fs::exists(path))
                        fs::create_directory(path);
                }else{
                    if(!ofs3.is_open()){
                        ofs3 = std::ofstream(outputDir + fileInfo.getRelativePath(), std::ios::out | std::ios::binary);
                        LOG.info("ServerSocket.receiveDirectory - file = " + outputDir + fileInfo.getRelativePath());
                        if(!ofs3.is_open())
                            throw FileException("ServerSocket.receiveDirectory - Cannot open/create received file on server");
                    }

                    ofs3.write(fileInfo.getContent().data(), fileInfo.getContent().size()*sizeof(char));
                    if(fileInfo.getContent().size() < Constants::Pipe::MAX_BYTE)
                        ofs3.close();
                }
            }
        }
        catch(FileException& exception){
            LOG.error(exception.getMessage());
        }
        catch(std::exception& exception){
            LOG.error(exception.what());
        }
    }

    void serverLoginCheck(){
        LOG.info("ServerSocket.serverLoginCheck");
        try{
            std::fstream serverFile;
            serverFile.open("../serverCredentials.txt");
            if(!serverFile.is_open()) {
                throw FileException("Error opening server credentials file");  //TODO crea file e inserire path master
            }
            else {
                std::vector<std::string> credentials; //TODO aprire json che ritorna oggetto parsificato
                std::string temp;
                std::string outputPath("/home/gaetano/CLionProjects/remote-backup/outputDirectory");
                while(std::getline(serverFile, temp)){
                    credentials.push_back(temp);
                }
                serverFile.clear();
                serverFile.seekg(0);
                auto receivedCredentials = receiveData<StringWrapper>(); //TODO usare classe Command
                auto content = receivedCredentials.getContent();
                std::string checkingString(content);
                std::replace(checkingString.begin(), checkingString.end(), '\n', ',');
                if(std::find(credentials.begin(), credentials.end(), checkingString) != credentials.end()) { //TODO cambiare condizione if a check esistenza cartella
                    LOG.info("ServerSocket.serverLoginCheck - Found User and Path");
                    /* credentials present */
                    std::vector<std::string> tempVector;
                    boost::algorithm::split(tempVector, content, boost::is_any_of("\n"));
                    std::replace(tempVector[1].begin(), tempVector[1].end(), '/', '_');
                    std::string path("/home/gaetano/CLionProjects/remote-backup/outputDirectory/" +
                                     tempVector[0] + "_" + tempVector[1]);
                    receiveDirectory(path);
                } else {
                    /* credentials not present */
                    LOG.info("ServerSocket.serverLoginCheck - User NOT found -> creation of user folder");
                    serverFile<<checkingString<<std::endl;

                    std::vector<std::string> tempVector;
                    boost::algorithm::split(tempVector, content, boost::is_any_of("\n"));
                    std::replace(tempVector[1].begin(), tempVector[1].end(), '/', '_');
                    std::string path("/home/gaetano/CLionProjects/remote-backup/outputDirectory/" +
                                     tempVector[0] + "_" + tempVector[1]);
                    fs::create_directory(path);
                    receiveDirectory(path);
                }
            }
            serverFile.close();
        }
        catch(FileException& exception){
            LOG.error(exception.getMessage()); // TODO sbagliato perchè viene lanciata nella stessa funzione
        }
        catch(std::exception& exception){
            LOG.error(exception.what());
        }
    }
};


#endif //REMOTE_BACKUP_SERVERSOCKET_H
