//
// Created by gaetano on 20/10/20.
//

#ifndef REMOTE_BACKUP_SERVERSOCKET_H
#define REMOTE_BACKUP_SERVERSOCKET_H

#include <cstdint>
#include <stdexcept>
#include <cstring>
#include "socket.h"
#include <algorithm>

class ServerSocket : private Socket{
    asio::ip::tcp::acceptor::endpoint_type end_type;
public:
    ServerSocket(){}
    ~ServerSocket(){
        socket.close();
    }

    void accept(int port) {
        try{
            asio::ip::tcp::acceptor acceptor_server(io_service,
                                                    asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
            std::cout<<"Waiting for incoming connections..."<<std::endl;
            acceptor_server.accept(socket, end_type); // Waiting for connection
            std::string sClientIp = end_type.address().to_string();
            unsigned short uiClientPort = end_type.port();
            std::cout<<sClientIp<<" connected on port: "<<uiClientPort<<std::endl;
        }
        catch(std::exception exception){
            std::cout << exception.what() << std::endl;
        }
    }

    void receiveDirectory(const std::string& outputDir){
        try{
            std::ofstream ofs3;
            while(true){
                auto fileInfo = receiveData<FileInfo>();

                // serve per bloccare la lettura da pipe/socket
                if(!fileInfo.isValid()) // TODO forse non serve se facciamo canale di controllo
                    throw FileException("Received file info is not valid");

                if(fileInfo.isDirectory()){
                    fs::path path(outputDir + fileInfo.getRelativePath());

                    if(!fs::exists(path))
                        fs::create_directory(path);
                }else{
                    if(!ofs3.is_open()){
                        ofs3 = std::ofstream(outputDir + fileInfo.getRelativePath(), std::ios::out | std::ios::binary);
                        if(!ofs3.is_open())
                            throw FileException("Cannot open/create received file on server");
                    }

                    ofs3.write(fileInfo.getContent().data(), fileInfo.getContent().size()*sizeof(char));
                    if(fileInfo.getContent().size() < Constants::Pipe::MAX_BYTE)
                        ofs3.close();
                }
            }
        }
        catch(FileException exception){
            std::cout << exception.getMessage() << std::endl;
        }
        catch(std::exception exception){
            std::cout << exception.what() << std::endl;
        }
    }

    void serverLoginCheck(){
        try{
            std::fstream serverFile;
            serverFile.open("/home/gaetano/CLionProjects/remote-backup/serverCredentials.txt");
            if(!serverFile.is_open()) {
                throw FileException("Error opening server credentials file");
            }
            else {
                std::vector<std::string> credentials;
                std::string temp;
                std::string outputPath("/home/gaetano/CLionProjects/remote-backup/outputDirectory");
                while(std::getline(serverFile, temp)){
                    std::cout<<"line: "<<temp<<std::endl;
                    credentials.push_back(temp);
                }
                serverFile.clear();
                serverFile.seekg(0);
                auto receivedCredentials = receiveData<StringWrapper>();
                auto content = receivedCredentials.getContent();
                std::string checkingString(content);
                std::replace(checkingString.begin(), checkingString.end(), '\n', ',');
                if(std::find(credentials.begin(), credentials.end(), checkingString) != credentials.end()) {
                    std::cout<<"Trovato user e path"<<std::endl;
                    /* credentials present */
                    std::vector<std::string> tempVector;
                    boost::algorithm::split(tempVector, content, boost::is_any_of("\n"));
                    std::replace(tempVector[1].begin(), tempVector[1].end(), '/', '_');
                    std::string path("/home/gaetano/CLionProjects/remote-backup/outputDirectory/" +
                                     tempVector[0] + "_" + tempVector[1]);
                    receiveDirectory(path);
                } else {
                    /* credentials not present */
                    std::cout<<"Non trovato. Creo user e path"<<std::endl;
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
        catch(FileException exception){
            std::cout << exception.getMessage() << std::endl;
        }
        catch(std::exception exception){
            std::cout << exception.what() << std::endl;
        }
    }
};


#endif //REMOTE_BACKUP_SERVERSOCKET_H
