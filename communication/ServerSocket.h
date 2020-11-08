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
#include "../models/FileConfig.h"
#include "../utils/StringUtils.h"


//TODO da levare ma da prendere logica login e modificarla
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
                auto fileChunk = receiveData<FileChunk>();

                // serve per bloccare la lettura da pipe/socket
                if(fileChunk.end()){
                    LOG.info("ServerSocket.receiveDirectory - finishReceiving");
                    return;
                }

                if(fileChunk.isDirectory()){
                    fs::path path(outputDir + fileChunk.getRelativePath());
                    LOG.info("ServerSocket.receiveDirectory - directory = " + path.string());

                    if(!fs::exists(path))
                        fs::create_directory(path);
                }else{
                    if(!ofs3.is_open()){
                        ofs3 = std::ofstream(outputDir + fileChunk.getRelativePath(), std::ios::out | std::ios::binary);
                        LOG.info("ServerSocket.receiveDirectory - file = " + outputDir + fileChunk.getRelativePath());
                        if(!ofs3.is_open())
                            throw FileException("ServerSocket.receiveDirectory - Cannot open/create received file on server");
                    }

                    ofs3.write(fileChunk.getContent().data(), fileChunk.getContent().size()*sizeof(char));
                    if(fileChunk.getContent().size() < Constants::Pipe::MAX_BYTE)
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
            auto receivedCredentials = receiveData<Command>();
            auto content = receivedCredentials.getMessage();
            std::vector<std::string> tempVector = StringUtils::split(content, "\n");
            std::replace(tempVector[1].begin(), tempVector[1].end(), '/', '_');
            std::string path(fileConfig.getOutputDirPath() + tempVector[0] + "_" + tempVector[1]);
            if(fs::exists(path)) {
                LOG.info("ServerSocket.serverLoginCheck - Found User and Path");
                /* credentials present */
                receiveDirectory(path);
            } else {
                /* credentials not present */
                LOG.info("ServerSocket.serverLoginCheck - User NOT found -> creation of user folder");

                fs::create_directory(path);
                receiveDirectory(path);
            }
        }
        catch(std::exception& exception){
            LOG.error(exception.what());
        }
    }
};


#endif //REMOTE_BACKUP_SERVERSOCKET_H
