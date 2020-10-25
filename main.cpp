#include <iostream>
#include <filesystem>
#include <boost/asio.hpp>
#include <vector>
#include <string>
#include "utils/Constants.h"
#include "models/FileInfo.h"
#include "communication/Pipe.h"
#include "utils/Logger.h"

namespace fs = std::filesystem;
namespace asio = boost::asio;

Logger LOG;

void startServer();
void startClient();

template <typename T>
T receive(asio::ip::tcp::socket& socket)
{
    ushort size = 0;
    socket.receive(asio::buffer(&size, sizeof(ushort)));
    size_t sizeRecv = 0;
    std::vector<char> buff(size);
    do{
        sizeRecv += socket.receive(asio::buffer(reinterpret_cast<char*>(&buff[sizeRecv]), (size - sizeRecv) * sizeof(char)));
    }while(sizeRecv < size);
    return Deserializer::deserialize<T>(buff);
}

template <typename T>
void send(asio::ip::tcp::socket& socket, T obj)
{
    auto buff = Serializer::serialize<T>(obj);
    ushort size = buff.size();
    socket.send(asio::buffer(&size, sizeof(ushort)));
    size_t sizeSent = 0;
    do{
        sizeSent += socket.send(asio::buffer(reinterpret_cast<char*>(&buff[sizeSent]), (size - sizeSent)*sizeof(char)));
    }while(sizeSent < size);
}


void sendFile(const std::string& basePath, const std::string& filePath, asio::ip::tcp::socket& socket)
{
    LOG.info("sendFile - filePath = " + filePath);
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
        send(socket, fileInfo);

        if(!ifs)
            break;
    }
    ifs.close();
}

void finishSending(asio::ip::tcp::socket& socket){
    LOG.info("finishSending");
    send(socket, FileInfo());
}

void createRemoteDirectory(const std::string& basePath, const std::string& directoryPath, asio::ip::tcp::socket& socket){
    LOG.info("createRemoteDirectory - directory = " + directoryPath);
    std::string relativePath = StringUtils::getStringDifference(directoryPath, basePath);
    FileInfo fileInfo("", directoryPath, relativePath);
    send(socket, fileInfo);
}

void sendDirectory(const std::string& directory, asio::ip::tcp::socket& socket){
    LOG.info("sendDirectory - directory = " + directory);
    fs::path path(directory);
    for(auto &p : fs::recursive_directory_iterator(path)){
        if( !fs::is_directory(p.status()) )
            sendFile(directory, p.path().string(), socket);
        else createRemoteDirectory(directory, p.path().string(), socket);
    }
    finishSending(socket);
}

void receiveDirectory(const std::string& outputDir, asio::ip::tcp::socket& socket){
    LOG.info("receiveDirectory - outputDir = " + outputDir);
    std::ofstream ofs3;
    while(true){
        auto fileInfo = receive<FileInfo>(socket);

        // serve per bloccare la lettura da pipe/socket
        if(!fileInfo.isValid()){
            LOG.info("receiveDirectory - finish receiving");
            return;
        } // TODO forse non serve se facciamo canale di controllo

        if(fileInfo.isDirectory()){
            fs::path path(outputDir + fileInfo.getRelativePath());
            LOG.info("receiveDirectory - receiving directory in = " + path.string());

            if(!fs::exists(path))
                fs::create_directory(path);
        }else{
            if(!ofs3.is_open()){
                std::string path(outputDir + fileInfo.getRelativePath());
                LOG.info("receiveDirectory - receiving file in = " + path);
                ofs3 = std::ofstream(path, std::ios::out | std::ios::binary);
                if(!ofs3.is_open()) throw std::exception(); // TODO da rivedere se lanciare eccezione, altro o eccezione custom
            }

            ofs3.write(fileInfo.getContent().data(), fileInfo.getContent().size()*sizeof(char));
            if(fileInfo.getContent().size() < Constants::Pipe::MAX_BYTE)
                ofs3.close();
        }
    }
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

void startServer(){
    LOG.info("startServer");
    std::string outputDirectory("/home/alessandro/CLionProjects/remote-backup/outputDirectory");
    asio::io_service io_service;

    asio::ip::tcp::acceptor::endpoint_type end_type;
    asio::ip::tcp::acceptor acceptor_server(io_service,asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 9999)); // Listening to incoming connection on port 9999
    asio::ip::tcp::socket server_socket(io_service); // Creating socket object

    LOG.info("startServer - Waiting for incoming connections...");
    acceptor_server.accept(server_socket, end_type); // Waiting for connection
    std::string sClientIp = end_type.address().to_string();
    unsigned short uiClientPort = end_type.port();
    LOG.info("startServer - " + sClientIp + " connected on port " + std::to_string(uiClientPort));

    receiveDirectory(outputDirectory, server_socket);
}

void startClient(){
    LOG.info("startClient");
    std::string inputDirectory("/home/alessandro/CLionProjects/remote-backup/inputDirectory");
    asio::io_service io_service;
    // socket creation
    asio::ip::tcp::socket client_socket(io_service);

    client_socket.connect(asio::ip::tcp::endpoint(asio::ip::address::from_string("0.0.0.0"),9999));

    sendDirectory(inputDirectory, client_socket);
}
