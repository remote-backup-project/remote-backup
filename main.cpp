#include <vector>
#include <string>
#include <thread>
#include "utils/Constants.h"
#include "utils/Logger.h"
#include "communication/Socket.h"
#include "communication/ServerSocket.h"
#include "models/FileConfig.h"

[[noreturn]] void startServer(){
    LOG.info("startServer");
    ServerSocket fileTransferSocket;
    ServerSocket commandSocket;
    fileConfig.readServerFile();

    std::vector<std::thread> threads;
    while(true){
        commandSocket.accept(Constants::Socket::COMMAND_PORT);
        auto command = commandSocket.receiveData<Command>();

        LOG.info("startServer - command = " + command.to_string());
        switch(command.getCode()){
            case Constants::Socket::START_TRANFER_FILE:
                threads.emplace_back([&fileTransferSocket, &commandSocket]()
                 {
                     fileTransferSocket.accept(Constants::Socket::FILE_TRANSFER_PORT);
                     fileTransferSocket.serverLoginCheck(fileConfig);
                     fileTransferSocket.close();
                 });
                commandSocket.sendData(Command(Constants::Socket::START_TRANFER_FILE, "Starting receiving files"));
                break;
            default:
                break;
        }
        commandSocket.close();
    }
//    fileTransferSocket.receiveDirectory(outputDirectory);
}

void startClient(){
    LOG.info("startClient");
    fileConfig.readClientFile();

    for(int i=0; i< 3; i++){
        // commandSocket creation
        Socket commandSocket;
        commandSocket.connect(Constants::Socket::SERVER_HOSTNAME, Constants::Socket::COMMAND_PORT);
        commandSocket.sendData(Command(Constants::Socket::START_TRANFER_FILE, "Starting sending files"));
        auto response = commandSocket.receiveData<Command>();
        LOG.info("startClient - command = " + response.to_string());

        std::vector<std::thread> threads;
        Socket fileTransferSocket;
        switch(response.getCode()){
            case Constants::Socket::START_TRANFER_FILE:
                threads.emplace_back([&fileTransferSocket]()
                 {
                     fileTransferSocket.connect(Constants::Socket::LOCAL_NETWORK, Constants::Socket::FILE_TRANSFER_PORT);
                     fileTransferSocket.doLogin(fileConfig);
                 });
                break;
            default:
                break;
        }

        for(auto &t : threads)
            if(t.joinable()) t.join();
    }

//    commandSocket.sendDirectory(inputDirectory);
}


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
