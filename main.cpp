#include <iostream>
#include <filesystem>
#include <arpa/inet.h>
#include "socket.h"
#include "serverSocket.h"

namespace fs = std::filesystem;

ServerSocket serverSocket(5000);

int main() {
    while(true){
        struct sockaddr_in addr;
        unsigned int len = sizeof(addr);
        std::cout<<"Waiting for incoming connections..."<<std::endl;
        Socket socket = serverSocket.accept(&addr, &len);
        char name[16];
        if(inet_ntop(AF_INET, &addr.sin_addr, name, sizeof(name)) == nullptr)
            throw std::runtime_error("Cannot convert address");
        std::cout<<"Got a connection from "<<name<<": "<<ntohs(addr.sin_port)<<"\n";

        char buffer[1024];
        int size = socket.read(buffer, sizeof(buffer)-1, 0);
        buffer[size] = 0;
        std::string str(buffer);
        std::cout<<"Received: "<<str<<std::endl;
        socket.write(buffer, size, 0);


        std::cout<<"Connection closed"<<std::endl;
    }
    return 0;
}
