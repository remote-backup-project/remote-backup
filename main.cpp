#include <iostream>
#include <filesystem>
#include <boost/asio.hpp>
#include <vector>
#include <string>
#include "utils/Constants.h"
#include "models/FileInfo.h"
#include "communication/Pipe.h"
#include "utils/Logger.h"
#include "socket.h"
#include "serverSocket.h"

namespace fs = std::filesystem;
namespace asio = boost::asio;

Logger LOG;

void startServer();
void startClient();


int main(int argc, char** argv) {
    if(argc != 2) return -1;
    int temp = atoi(argv[1]);
    if(temp == 0){
        startServer();
    }else{
        startClient();
    }
    return 0;
}

void startServer(){
    LOG.info("startServer");
    std::string outputDirectory("/home/gaetano/CLionProjects/remote-backup/outputDirectory");
    ServerSocket serverSocket;
    serverSocket.accept(9999);
    serverSocket.serverLoginCheck();

//    serverSocket.receiveDirectory(outputDirectory);
}

void startClient(){
    LOG.info("startClient");
    std::string inputDirectory("/home/alessandro/CLionProjects/remote-backup/inputDirectory");
    asio::io_service io_service;
    // socket creation
    Socket socket;
    socket.connect("0.0.0.0", 9999);
    socket.doLogin();
//    socket.sendDirectory(inputDirectory);
}
