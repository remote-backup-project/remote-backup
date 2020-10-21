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
public:
    ServerSocket(uint16_t portNumber) {
        struct sockaddr_in sockAddrIn;
        sockAddrIn.sin_family = AF_INET;
        sockAddrIn.sin_port = htons(portNumber);
        sockAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
        if(::bind(sockFd, reinterpret_cast<struct sockaddr*>(&sockAddrIn), sizeof(sockAddrIn)) != 0)
            throw std::runtime_error("Cannot bind port");
        if(::listen(sockFd,8) != 0){
            throw std::runtime_error("Cannot listen port");
        }
        char name[16];
        inet_ntop(AF_INET, &sockAddrIn.sin_addr, name, sizeof(name));
        std::cout<<"Server socket opened on "<<name<<": "<<ntohs(sockAddrIn.sin_port)<<std::endl;
    }
    ~ServerSocket(){
        if (sockFd != 0){
            close(sockFd);
            std::cout<<"Server socket "<<sockFd<<" closed"<<std::endl;
        }
    }

    Socket accept(struct sockaddr_in* addr, unsigned int* len) {
        int fd = ::accept(sockFd, reinterpret_cast<struct sockaddr*>(&addr), len);
        if(fd < 0) throw std::runtime_error("Cannot accept socket");
        return Socket(fd);
    }
};


#endif //REMOTE_BACKUP_SERVERSOCKET_H
