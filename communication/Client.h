//
// Created by alessandro on 07/11/20.
//

#ifndef REMOTE_BACKUP_CLIENT_H
#define REMOTE_BACKUP_CLIENT_H

#include <string>
#include <boost/asio.hpp>
#include "ClientConnection.h"

class Client{
public:
    Client();

    void run();

private:

    ClientConnectionPtr clientConnectionPtr;

    boost::asio::io_context ioContext;

    void sendFile(const std::string& basePath, const std::string& filePath);

    void createRemoteDirectory(const std::string& basePath, const std::string& directoryPath);

    void sendDirectory(const std::string& directory);

    void sendData(std::string uri, std::string body);
};

#endif //REMOTE_BACKUP_CLIENT_H
