#ifndef REMOTE_BACKUP_CLIENT_H
#define REMOTE_BACKUP_CLIENT_H

#include <string>
#include <boost/asio.hpp>
#include "ClientConnection.h"
#include "../models/FileWatcher.h"

class Client{
public:
    Client();

    void run();

private:

    ClientConnectionPtr clientConnectionPtr;

    boost::asio::io_context ioContext;

    boost::asio::signal_set signals;

    FileWatcher fileWatcher;

    std::string macAddress;

    void sendHashFile(const std::string& filePath);

    void createRemoteDirectory(const std::string& directoryPath);

    void sendDirectory();

    void handleStop();

    static void sendContentFile(void* client, std::string filePath);

    static void sendRequest(void* client, std::string uri, std::string body, void (* onResponse)(void*, std::string));

    void watchFileSystem();

    void setMacAddress();
};

#endif //REMOTE_BACKUP_CLIENT_H
