#include <iostream>
#include <filesystem>
#include <arpa/inet.h>
#include <netdb.h>
#include "socket.h"
#include "serverSocket.h"
#include <boost/asio.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;

void serverFunction();
void clientFunction();
void serverFunction2();
void clientFunction2();

std::string getDataFromBuffer(tcp::socket& socket)
{
    streambuf buf;
    read_until(socket, buf, "\n");
    std::string data = buffer_cast<const char*>(buf.data());
    return data;
}
void sendDataToBuffer(tcp::socket& socket, const std::string& message)
{
    write(socket,buffer(message + "\n"));
}

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    if(argc != 2) return -1;
    int temp = atoi(argv[1]);
    if(temp == 0){
        std::cout<<"SERVER FUNCTION"<<std::endl;
        serverFunction2();
    }
    else{
        clientFunction2();
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

void serverFunction2(){
    io_service io_service;

    tcp::acceptor::endpoint_type end_type;
    tcp::acceptor acceptor_server(io_service,tcp::endpoint(tcp::v4(), 9999)); // Listening to incoming connection on port 9999
    tcp::socket server_socket(io_service); // Creating socket object

    std::cout<<"Waiting for incoming connections..."<<std::endl;
    acceptor_server.accept(server_socket, end_type); // Waiting for connection
    std::string sClientIp = end_type.address().to_string();
    unsigned short uiClientPort = end_type.port();
    std::cout<<sClientIp<<" connected on port: "<<uiClientPort<<std::endl;

    while (true) {
        std::string readString = getDataFromBuffer(server_socket);
        readString.pop_back();

        std::string reply;
        reply = "Reply to message: " + readString;
        std::cout << "Server: " << reply << std::endl;
        sendDataToBuffer(server_socket, reply);

        // Validating if the connection has to be closed
        if (readString == "stop_server") {
            std::cout<<"Server stopped"<<std::endl;
            break;
        }
    }
}
void clientFunction2(){
    io_service io_service;
    // socket creation
    ip::tcp::socket client_socket(io_service);

    client_socket.connect(tcp::endpoint(address::from_string("127.0.0.1"),9999));

    while (true) {
        std::cout << "Enter message: ";
        std::string message, reply;
        getline(std::cin, message);
        sendDataToBuffer(client_socket, message);

        reply = getDataFromBuffer(client_socket);
        reply.pop_back(); // Popping last character "\n"
        // Validating if the connection has to be closed
        if (message == "stop_client") {
            std::cout << "Connection terminated" << std::endl;
            break;
        }
        std::cout << "Server: " << reply << std::endl;
    }
}
