//
// Created by gaetano on 20/10/20.
//

#ifndef REMOTE_BACKUP_SOCKET_H
#define REMOTE_BACKUP_SOCKET_H

#include <sys/socket.h>
#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

class Socket {
    int sockFd;
    Socket(const Socket&) = delete;
    Socket& operator = (const Socket&) = delete;
    Socket(int sockFd): sockFd(sockFd){}

    friend class ServerSocket;
public:
    Socket() {
        sockFd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sockFd < 0) throw std::runtime_error("Unable to create socket");
        std::cout<<"Socket "<<sockFd<<" created"<<std::endl;
    }
    ~Socket() {
        if (sockFd != 0){
            close(sockFd);
            std::cout<<"Socket "<<sockFd<<" closed"<<std::endl;
        }
    }
    Socket(Socket &&other) : sockFd(other.sockFd){
        other.sockFd = 0;
    }
    Socket &operator = (Socket &&other){
        if(sockFd != 0) close(sockFd);
        sockFd = other.sockFd;
        other.sockFd = 0;
        return *this;
    }
    ssize_t  read(char *buffer, size_t len, int options){
        ssize_t  res = recv(sockFd, buffer, len, options);
        if(res < 0) throw std::runtime_error("Cannot receive from socket");
        return res;
    }
    ssize_t write(char *buffer, size_t len, int options){
        ssize_t  res = send(sockFd, buffer, len, options);
        if(res < 0) throw std::runtime_error("Cannot write on socket");
        return res;
    }
    void connect(struct sockaddr_in *addr, unsigned int len){
        if(::connect(sockFd, reinterpret_cast<struct sockaddr*>(addr), len) != 0)
            throw new std::runtime_error("Cannot connect to remote socket");
    }
};


#endif //REMOTE_BACKUP_SOCKET_H
