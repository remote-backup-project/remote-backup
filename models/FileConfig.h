#ifndef REMOTE_BACKUP_FILECONFIG_H
#define REMOTE_BACKUP_FILECONFIG_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <filesystem>
#include "Serializable.h"
#include "../utils/StringUtils.h"
#include "../converters/Deserializer.h"
#include "../converters/Serializer.h"

namespace fs = std::filesystem;

class FileConfig : Serializable{
    std::string inputDirPath;
    std::string outputDirPath;
    std::string username;
    std::string hostname;
    std::string port;
    std::string macInterface;

    void createServerConfigFile();

    void createClientConfigFile();

public:
    FileConfig();
    FileConfig(std::string inputDirPath, std::string outputDirPath, std::string username, std::string hostname, std::string port);
    FileConfig(std::string inputDirPath, std::string outputDirPath, std::string username, std::string hostname, std::string port, std::string macInterface);

    std::string getInputDirPath();

    std::string getOutputDirPath();

    std::string getUsername();

    std::string getHostname();

    std::string getPort();

    std::string getMacInterface();


    void writeAsString(boost::property_tree::ptree& pt) override;

    void readAsString(boost::property_tree::ptree& pt) override;

    void readServerFile();

    void readClientFile();

    std::string to_string() override;
};

extern FileConfig fileConfig;

#endif //REMOTE_BACKUP_FILECONFIG_H