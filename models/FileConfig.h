//
// Created by gaetano on 02/11/20.
//

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

namespace fs = std::filesystem;

class FileConfig : Serializable{
    std::string inputDirPath;
    std::string outputDirPath;
    std::string username;
    std::string hostname;

    void createServerConfigFile(){
        std::fstream configFile;
        std::string hostname, outputDirPath;
        std::cout << "Inserire hostname: " << std::endl;
        getline(std::cin, hostname);
        std::cout << "Inserire path della output directory: " << std::endl;
        getline(std::cin, outputDirPath);
        FileConfig temp("", outputDirPath, "", hostname);
        auto res = Serializer::serialize(temp);
        configFile.open("../config.json", std::fstream::out);
        configFile << res.data();
        configFile.close();

        if(!fs::exists(outputDirPath))
            fs::create_directory(outputDirPath);

        readServerFile();
    }

    void createClientConfigFile(){
        std::fstream configFile;
        std::string username, inputDirPath;
        std::cout << "Inserire username: " << std::endl;
        getline(std::cin, username);
        std::cout << "Inserire path della input directory: " << std::endl;
        getline(std::cin, inputDirPath);
        FileConfig temp(inputDirPath, "", username, "");
        auto res = Serializer::serialize(temp);
        configFile.open("../config.json", std::fstream::out);
        configFile << res.data();
        configFile.close();

        readClientFile();
    }

public:
    FileConfig(){};
    FileConfig(std::string inputDirPath, std::string outputDirPath, std::string username, std::string hostname):
            inputDirPath(std::move(inputDirPath)),
            outputDirPath(std::move(outputDirPath)),
            username(std::move(username)),
            hostname(std::move(hostname)){};

    std::string getInputDirPath(){ return inputDirPath; }
    std::string getOutputDirPath(){ return outputDirPath; }
    std::string getUsername() { return username; }
    std::string getHostname() { return hostname; }

    void writeAsString(boost::property_tree::ptree& pt){
        pt.put("inputDirPath", StringUtils::encodeBase64(this->inputDirPath));
        pt.put("outputDirPath", this->outputDirPath);
        pt.put("username", this->username);
        pt.put("hostname", this->hostname);
    }

    void readAsString(boost::property_tree::ptree& pt){
        this->inputDirPath = pt.get<std::string>("inputDirPath");
        this->outputDirPath = pt.get<std::string>("outputDirPath");
        this->username = pt.get<std::string>("username");
        this->hostname = pt.get<std::string>("hostname");
    }

    void readServerFile(){
        std::fstream configFile;
        configFile.open("../config.json", std::fstream::in);

        if(!configFile.is_open()){
            createServerConfigFile();
        }
        else {
            std::ostringstream tmp;
            tmp << configFile.rdbuf();
            std::string tempString = tmp.str();
            std::vector<char> vector(tempString.begin(), tempString.end());
            auto configData = Deserializer::deserialize<FileConfig>(vector);

            if(configData.getOutputDirPath().empty() || configData.getHostname().empty()){
                configFile.close();
                createServerConfigFile();
            }
            else{
                this->inputDirPath = configData.getInputDirPath();
                this->outputDirPath = configData.getOutputDirPath();
                this->username = configData.getUsername();
                this->hostname = configData.getHostname();
                configFile.close();
            }
        }
    }

    void readClientFile(){
        std::fstream configFile;
        configFile.open("../config.json", std::fstream::in);

        if(!configFile.is_open()){
            createClientConfigFile();
        }
        else {
            std::ostringstream tmp;
            tmp << configFile.rdbuf();
            std::string tempString = tmp.str();
            std::vector<char> vector(tempString.begin(), tempString.end());
            auto configData = Deserializer::deserialize<FileConfig>(vector);

            if(configData.getInputDirPath().empty() || configData.getUsername().empty()){
                configFile.close();
                createClientConfigFile();
            }
            else{
                this->inputDirPath = configData.getInputDirPath();
                this->outputDirPath = configData.getOutputDirPath();
                this->username = configData.getUsername();
                this->hostname = configData.getHostname();
                configFile.close();
            }
        }
    }
};

FileConfig fileConfig;

#endif //REMOTE_BACKUP_FILECONFIG_H
