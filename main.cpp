#include <iostream>
#include <filesystem>
#include <boost/asio.hpp>
#include <vector>
#include <string>
#include "utils/Constants.h"
#include "models/FileInfo.h"
#include "communication/Pipe.h"
#include "socket.h"
#include "serverSocket.h"

namespace fs = std::filesystem;
namespace asio = boost::asio;

void startServer();
void startClient();


int main(int argc, char** argv) {
    int i;
    if(argc != 2) return -1;
    int temp = atoi(argv[1]);
    if(temp == 0){
        std::cout<<"SERVER FUNCTION\n"<<std::endl;
        startServer();
    }else{
        std::cout<<"CLIENT FUNCTION\n"<<std::endl;
        startClient();
    }
    return 0;
}

void startServer(){
    std::string outputDirectory("/home/alessandro/CLionProjects/remote-backup/outputDirectory");
    ServerSocket serverSocket();
//    serverSocket.accept(9999);
//
//    receiveDirectory(outputDirectory);
}

void startClient(){
    std::string inputDirectory("/home/alessandro/CLionProjects/remote-backup/inputDirectory");
    asio::io_service io_service;
    // socket creation
    Socket socket;
//    socket.connect("0.0.0.0", 9999);
//    socket.sendDirectory(inputDirectory);
}
