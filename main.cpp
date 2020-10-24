#include <iostream>
#include <filesystem>
#include <boost/asio.hpp>
#include <vector>
#include <string>
#include "utils/Constants.h"
#include "models/FileInfo.h"
#include "communication/Pipe.h"

namespace fs = std::filesystem;
using namespace boost::asio;
using namespace boost::asio::ip;

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
    write(socket,buffer(message + "\n\n"));
}


void sendFile(const std::string& basePath, const std::string& filePath, Pipe& pipe)
{
    std::string v2;
    std::ifstream ifs;
    ifs.open(filePath, std::ios::in | std::ios::binary);

    if(!ifs.is_open())
        throw std::exception(); // TODO da rivedere se lanciare eccezione, altro o eccezione custom

    std::vector<char> data(Constants::Pipe::MAX_BYTE + 1,0);
    while (true)
    {
        ifs.read(data.data(), Constants::Pipe::MAX_BYTE);
        std::streamsize s = ((ifs)? Constants::Pipe::MAX_BYTE : ifs.gcount());
        data[s] = 0;
        FileInfo fileInfo(std::string(data.data(), s), filePath, StringUtils::getStringDifference(filePath, basePath));
        pipe.write_pipe(fileInfo); // TODO sostituire con socket

        if(!ifs)
            break;
    }
    ifs.close();
}

void finishSending(Pipe& pipe){
    pipe.write_pipe(FileInfo()); //TODO sostituire con socket
}

void createRemoteDirectory(const std::string& basePath, const std::string& directoryPath, Pipe& pipe){
    std::string relativePath = StringUtils::getStringDifference(directoryPath, basePath);
    FileInfo fileInfo("", directoryPath, relativePath);
    pipe.write_pipe(fileInfo); // TODO sostituire con socket
}

void sendDirectory(const std::string& directory, Pipe& pipe){
    fs::path path(directory);
    for(auto &p : fs::recursive_directory_iterator(path)){
        std::cout << "sending = " + p.path().string() << std::endl;
        if( !fs::is_directory(p.status()) ){
            sendFile(directory, p.path().string(), pipe);
        }else createRemoteDirectory(directory, p.path().string(), pipe);
    }
    finishSending(pipe); // TODO forse non serve se facciamo canale di controllo
}

void receiveDirectory(const std::string& outputDir, Pipe& pipe){
    std::ofstream ofs3;
    while(true){
        auto fileInfo = pipe.read_pipe<FileInfo>();

        // serve per bloccare la lettura da pipe/socket
        if(!fileInfo.isValid()) // TODO forse non serve se facciamo canale di controllo
            return;

        if(fileInfo.isDirectory()){
            fs::path path(outputDir + fileInfo.getRelativePath());

            if(!fs::exists(path))
                fs::create_directory(path);
        }else{
            if(!ofs3.is_open()){
                ofs3 = std::ofstream(outputDir + fileInfo.getRelativePath(), std::ios::out | std::ios::binary);
                if(!ofs3.is_open()) throw std::exception(); // TODO da rivedere se lanciare eccezione, altro o eccezione custom
            }

            ofs3.write(fileInfo.getContent().data(), fileInfo.getContent().size()*sizeof(char));
            if(fileInfo.getContent().size() < Constants::Pipe::MAX_BYTE)
                ofs3.close();
        }
    }
}

int main() {
    Pipe pipe{};
    std::string inputDirectory("/home/alessandro/CLionProjects/remote-backup/inputDirectory");
    std::string outputDirectory("/home/alessandro/CLionProjects/remote-backup/outputDirectory");

    if(pipe.openConnection().hasErrors())
        return 1;

    switch(pipe.getPid()){
        case Constants::Pipe::CHILD:
            pipe.closeChildInRead();
            sendDirectory(inputDirectory, pipe);
            exit(1);
        default:
            pipe.closeParentInWrite();
            receiveDirectory(outputDirectory, pipe);
    }

    return 0;
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

    client_socket.connect(tcp::endpoint(address::from_string("0.0.0.0"),9999));

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
