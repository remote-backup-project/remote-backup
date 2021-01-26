#include "Client.h"
#include <ostream>
#include <string>
#include <filesystem>
#include <utility>
#include "../utils/Logger.h"
#include "../utils/Constants.h"
#include "../utils/StringUtils.h"
#include "../models/FileConfig.h"

using boost::asio::ip::tcp;
namespace fs = std::filesystem;
namespace asio = boost::asio;


Client::Client():
    clientConnectionPtr(),
    signals(ioContext)
{
    signals.add(SIGINT);
    signals.add(SIGTERM);
#if defined(SIGQUIT)
    signals.add(SIGQUIT);
#endif
    signals.async_wait(boost::bind(&Client::handleStop, this));

    setMacAddress();
    fileConfig.readClientFile();
//    sendDirectory();
    watchFileSystem();
}

void Client::run(){
    ioContext.run();
}

void Client::handleStop()
{
    fileWatcher.stop();
    ioContext.stop();
    LOG.info("Client::handleStop - Client stopped");
}


void Client::sendHashFile(const std::string& filePath)
{
    LOG.debug("Client::sendHashFile - filePath = " + filePath);
    try{
        std::string md5FileTuple = StringUtils::md5FromFile(filePath);
        std::uintmax_t fileSize = fs::file_size(fs::path(filePath));
        FileChunk fileChunk(0, md5FileTuple, "", filePath, StringUtils::getStringDifference(filePath, fileConfig.getInputDirPath()), fileSize);
        sendRequest(this, Services::CHECKSUM_FILE, fileChunk.to_string(), sendContentFile);
    }
    catch(std::exception& exception){
        LOG.error("Client::sendHashFile - " + std::string(exception.what()));
    }
}

void Client::createRemoteDirectory(const std::string& directoryPath){
    LOG.debug("Client::createRemoteDirectory - directoryPath = " + directoryPath);
    std::string relativePath = StringUtils::getStringDifference(directoryPath, fileConfig.getInputDirPath());
    FileChunk fileChunk(directoryPath, relativePath);
    sendRequest(this, Services::TRANSFER_DIRECTORY, fileChunk.to_string(), [](void*, std::string){});
}

void Client::sendDirectory(){
    LOG.debug("Client::sendDirectory - directory = " + fileConfig.getInputDirPath());
    fs::path path(fileConfig.getInputDirPath());
    for(auto &p : fs::recursive_directory_iterator(path)){
        if( !fs::is_directory(p.status()) ){
            sendHashFile(p.path().string());
        }else createRemoteDirectory(p.path().string());
    }
}

void Client::sendRequest(void *context, std::string uri, std::string body, void (* onResponse)(void*, std::string))
{
    LOG.info("Client::sendRequest - URI = < " + uri + " >");
    auto* client = reinterpret_cast<Client*>(context);
    Request request(std::move(uri), std::move(body));
    Header header(Config::MAC_ADDR, client->macAddress);
    request.addHeader(header);

    client->clientConnectionPtr.reset(new ClientConnection(client->ioContext));
    client->clientConnectionPtr->setRequest(request);
    client->clientConnectionPtr->setCallback(context, onResponse);
    client->clientConnectionPtr->start();
}

void Client::sendContentFile(void* client, std::string filePath)
{
    LOG.debug("Client::sendContentFile - filePath = " + filePath);
    try{
        std::ifstream ifs(filePath, std::ios::in | std::ios::binary);

        if(!ifs.is_open()){
            LOG.error("Client::sendContentFile - cannot open file <" + filePath + ">");
            return;
        }

        std::vector<char> data(Socket::CHUNK_SIZE + 1, 0);
        std::uintmax_t fileSize = fs::file_size(fs::path(filePath));
        std::string md5FileTuple = StringUtils::md5FromFile(filePath);

        long chunk = 1;
        while (ifs)
        {
            ifs.read(data.data(), Socket::CHUNK_SIZE);
            std::streamsize s = ifs.gcount();
            data[s] = 0;
            FileChunk fileChunk(chunk++, md5FileTuple, std::string(data.data(), s), filePath, StringUtils::getStringDifference(filePath, fileConfig.getInputDirPath()), fileSize);
            sendRequest(client, Services::TRANSFER_FILE, fileChunk.to_string(), [](void*, std::string){});
        }

        ifs.close();

        auto* clientPtr = reinterpret_cast<Client*>(client);
        clientPtr->sendHashFile(filePath);
    }
    catch(std::exception& exception){
        LOG.error("Client::sendContentFile - " + std::string(exception.what()));
    }
}

void Client::watchFileSystem()
{
    fileWatcher.start(ioContext, [&] (std::string news_on_path, FileWatcherStatus::FileStatus status) -> void {

        switch(status) {
            case FileWatcherStatus::FileStatus::CREATED:
                if(fs::is_directory(fs::path(news_on_path)))
                {
                    LOG.debug("Client::watchFileSystem - created directory : " + news_on_path);
                    createRemoteDirectory(news_on_path);
                }
                else
                {
                    LOG.debug("Client::watchFileSystem - created file : " + news_on_path);
                    sendHashFile(news_on_path);
                }
                break;
            case FileWatcherStatus::FileStatus::MODIFIED:
                if(!fs::is_directory(fs::path(news_on_path)))
                {
                    LOG.debug("Client::watchFileSystem - modified file : " + news_on_path);
                    sendHashFile(news_on_path);
                }
                break;
            case FileWatcherStatus::FileStatus::ERASED:
            {
                LOG.debug("Client::watchFileSystem - deleted : " + news_on_path);
                FileChunk fileChunk(StringUtils::getStringDifference(news_on_path, fileConfig.getInputDirPath()));
                sendRequest(this, Services::DELETE_RESOURCE, fileChunk.to_string(), [](void*, std::string){});
                break;
            }
            default:
                LOG.error("Client::watchFileSystem - Unknown file status < " + std::to_string((int)status) + " >");
                break;
        }
    });
}

void Client::setMacAddress()
{
    int fd;

    struct ifreq ifr;
    char *iface = fileConfig.getMacInterface().data();
    char *mac;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy((char *)ifr.ifr_name , (const char *)iface , IFNAMSIZ-1);

    ioctl(fd, SIOCGIFHWADDR, &ifr);

    close(fd);

    mac = (char *)ifr.ifr_hwaddr.sa_data;

    //display mac address
    char uc_Mac[32] = {0};
    sprintf((char *)uc_Mac,(const char *)"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x" ,
            mac[0] & 0xff, mac[1] & 0xff, mac[2] & 0xff, mac[3] & 0xff, mac[4] & 0xff, mac[5] & 0xff);

    macAddress = std::string(uc_Mac);
}
