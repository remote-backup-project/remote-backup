//
// Created by alessandro on 31/10/20.
//

#ifndef REMOTE_BACKUP_COMMAND_H
#define REMOTE_BACKUP_COMMAND_H

#include <iostream>
#include <string>
#include <utility>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "Serializable.h"

class Command: Serializable{
    int code;
    std::string message;
public:

    Command(){}
    Command(int code, std::string message): code(code), message(std::move(message)){}

    int getCode(){return code;}
    std::string getMessage(){return message;}

    void writeAsString(boost::property_tree::ptree& pt){
        pt.put("code", this->code);
        pt.put("message", this->message);
    }

    void readAsString(boost::property_tree::ptree& pt){
        this->code = pt.get<int>("code");
        this->message = pt.get<std::string>("message");
    }

    std::string to_string(){
        auto string = Serializer::serialize(*this);
        return std::string(string.begin(), string.end());
    }
};

#endif //REMOTE_BACKUP_COMMAND_H
