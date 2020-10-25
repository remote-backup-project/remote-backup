//
// Created by gaetano on 20/10/20.
//

#ifndef REMOTE_BACKUP_SERVERSOCKET_H
#define REMOTE_BACKUP_SERVERSOCKET_H

#include <cstdint>
#include <sys/socket.h>
#include <stdexcept>
#include <cstring>
#include <netinet/in.h>
#include <zconf.h>
#include <arpa/inet.h>
#include "socket.h"

class ServerSocket : private Socket{
//    asio::ip::tcp::acceptor* acceptor_server;
    asio::ip::tcp::acceptor::endpoint_type end_type;
public:
    ServerSocket(){
        std::cout<<"Server socket opened on "<< socket.local_endpoint().address().to_string()
            <<": "<< socket.local_endpoint().port() <<std::endl;
    }
    ~ServerSocket(){
        socket.close();
    }

//    void accept(int port) {
//        acceptor_server = asio::ip::tcp::acceptor(io_service,
//            asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
//        std::cout<<"Waiting for incoming connections..."<<std::endl;
//        acceptor_server.accept(socket, end_type); // Waiting for connection
//        std::string sClientIp = end_type.address().to_string();
//        unsigned short uiClientPort = end_type.port();
//        std::cout<<sClientIp<<" connected on port: "<<uiClientPort<<std::endl;
//    }
};


#endif //REMOTE_BACKUP_SERVERSOCKET_H
