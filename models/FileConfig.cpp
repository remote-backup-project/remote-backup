#include "FileConfig.h"

FileConfig::FileConfig(){};

FileConfig::FileConfig(std::string inputDirPath, std::string outputDirPath, std::string username, std::string hostname, std::string port):
        inputDirPath(std::move(inputDirPath)),
        outputDirPath(std::move(outputDirPath)),
        username(std::move(username)),
        hostname(std::move(hostname)),
        port(std::move(port)){};

FileConfig::FileConfig(std::string inputDirPath, std::string outputDirPath, std::string username, std::string hostname, std::string port, std::string macInterface):
        inputDirPath(std::move(inputDirPath)),
        outputDirPath(std::move(outputDirPath)),
        username(std::move(username)),
        hostname(std::move(hostname)),
        port(std::move(port)),
        macInterface(std::move(macInterface)){};

void FileConfig::createServerConfigFile(){
    std::fstream configFile;
    std::string hostname, outputDirPath, port;
    std::cout << "Insert hostname: " << std::endl;
    getline(std::cin, hostname);
    std::cout << "Insert path of the output directory: " << std::endl;
    getline(std::cin, outputDirPath);
    std::cout << "Insert port: " << std::endl;
    getline(std::cin, port);
    FileConfig temp("", outputDirPath, "", hostname, port);
    auto res = Serializer::serialize(temp);
    configFile.open("../config.json", std::fstream::out);
    configFile << res.data();
    configFile.close();

    if(!fs::exists(outputDirPath))
        fs::create_directory(outputDirPath);

    readServerFile();
}

void FileConfig::createClientConfigFile(){
    std::fstream configFile;
    std::string username, inputDirPath, port, macInterface;
    std::cout << "Insert username: " << std::endl;
    getline(std::cin, username);
    std::cout << "Insert path of the input directory: " << std::endl;
    getline(std::cin, inputDirPath);
    std::cout << "Insert port: " << std::endl;
    getline(std::cin, port);
    std::cout << "Insert mac interface: " << std::endl;
    getline(std::cin, macInterface);
    FileConfig temp(inputDirPath, "", username, "", port, macInterface);
    auto res = Serializer::serialize(temp);
    configFile.open("../config.json", std::fstream::out);
    configFile << res.data();
    configFile.close();

    readClientFile();
}

std::string FileConfig::getInputDirPath(){ return inputDirPath; }
std::string FileConfig::getOutputDirPath(){ return outputDirPath; }
std::string FileConfig::getUsername() { return username; }
std::string FileConfig::getHostname() { return hostname; }
std::string FileConfig::getPort(){ return port; };
std::string FileConfig::getMacInterface() { return macInterface; }

void FileConfig::writeAsString(boost::property_tree::ptree& pt){
    pt.put("inputDirPath", this->inputDirPath);
    pt.put("outputDirPath", this->outputDirPath);
    pt.put("username", this->username);
    pt.put("hostname", this->hostname);
    pt.put("port", this->port);
    pt.put("macInterface", this->macInterface);
}

void FileConfig::readAsString(boost::property_tree::ptree& pt){
    this->inputDirPath = pt.get<std::string>("inputDirPath");
    this->outputDirPath = pt.get<std::string>("outputDirPath");
    this->username = pt.get<std::string>("username");
    this->hostname = pt.get<std::string>("hostname");
    this->port = pt.get<std::string>("port");
    this->macInterface = pt.get<std::string>("macInterface");
}

void FileConfig::readServerFile(){
    std::fstream configFile;
    configFile.open("../config.json", std::fstream::in);

    if(!configFile.is_open()){
        createServerConfigFile();
    }
    else {
        std::ostringstream tmp;
        tmp << configFile.rdbuf();
        auto configData = Deserializer::deserialize<FileConfig>(tmp.str());

        if(configData.getOutputDirPath().empty() || configData.getHostname().empty() || configData.getPort().empty()){
            configFile.close();
            createServerConfigFile();
        }
        else{
            this->inputDirPath = configData.getInputDirPath();
            this->outputDirPath = configData.getOutputDirPath();
            this->username = configData.getUsername();
            this->hostname = configData.getHostname();
            this->port = configData.getPort();
            configFile.close();
        }
    }
}

void FileConfig::readClientFile(){
    std::fstream configFile;
    configFile.open("../config.json", std::fstream::in);

    if(!configFile.is_open()){
        createClientConfigFile();
    }
    else {
        std::ostringstream tmp;
        tmp << configFile.rdbuf();
        auto configData = Deserializer::deserialize<FileConfig>(tmp.str());

        if(configData.getInputDirPath().empty() || configData.getUsername().empty() || configData.getPort().empty() || configData.getMacInterface().empty()){
            configFile.close();
            createClientConfigFile();
        }
        else{
            this->inputDirPath = configData.getInputDirPath();
            this->outputDirPath = configData.getOutputDirPath();
            this->username = configData.getUsername();
            this->hostname = configData.getHostname();
            this->port = configData.getPort();
            this->macInterface = configData.getMacInterface();
            configFile.close();
        }
    }
}

std::string FileConfig::to_string(){
    auto string = Serializer::serialize(*this);
    return std::string(string.begin(), string.end());
}

FileConfig fileConfig;