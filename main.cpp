#include <iostream>
#include <filesystem>
#include <arpa/inet.h>
#include <netdb.h>
#include "socket.h"
#include "serverSocket.h"

void serverFunction();
void clientFunction();

namespace fs = std::filesystem;

bool isServer = false;

int main(int argc, char** argv) {
    if(argc != 2) return -1;
    int temp = atoi(argv[1]);
    if(temp == 0){
        serverFunction();
    }
    else{
        clientFunction();
    }
    return 0;
}

void serverFunction(){
    ServerSocket serverSocket(5000);
    while(true){
        struct sockaddr_in addr;
        unsigned int len = sizeof(addr);
        std::cout<<"Waiting for incoming connections..."<<std::endl;
        Socket socket = serverSocket.accept(&addr, &len);

        char name[16];
        if(inet_ntop(AF_INET, &addr.sin_addr, name, sizeof(name)) == nullptr)
            throw std::runtime_error("Cannot convert address");
        std::cout<<"Got a connection from "<<name<<": "<<ntohs(addr.sin_port)<<"\n"<<std::endl;

        char buffer[1024];
        int size = socket.read(buffer, sizeof(buffer)-1, 0);
        buffer[size] = 0;
        std::string str(buffer);
        std::cout<<"Received: "<<str<<std::endl;

        std::string temp("Returned_response");
        socket.write(const_cast<char *>(temp.c_str()), temp.size(), 0);

        std::cout<<"\nConnection closed"<<std::endl;
    }
}
void clientFunction(){
    Socket clientSocket;
    struct sockaddr_in destAddr;
    struct hostent* server;
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &destAddr, sizeof(destAddr)); //azzero destAddr struct

    destAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&destAddr.sin_addr.s_addr, server->h_length); //copio l'indirizzo del serve nella mia struttura
    destAddr.sin_port = htons(5000);
    clientSocket.connect(&destAddr, sizeof(destAddr));
    printf("Please enter the message: ");
    char buffer[256];
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    clientSocket.write(buffer, strlen(buffer), 0);
    bzero(buffer,256);
    clientSocket.read(buffer, 255, 0);
    printf("%s\n", buffer);
}
