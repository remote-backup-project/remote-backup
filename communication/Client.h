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

    void sendHashFile(const std::string& filePath);

    void createRemoteDirectory(const std::string& directoryPath);

    void sendDirectory();

    static void sendContentFile(void* client, std::string filePath);

    static void sendRequest(void* client, std::string uri, std::string body, void (* onResponse)(void*, std::string));
};

#endif //REMOTE_BACKUP_CLIENT_H
