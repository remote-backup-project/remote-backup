//
// Created by alessandro on 02/11/20.
//

#ifndef REMOTE_BACKUP_HEADER_H
#define REMOTE_BACKUP_HEADER_H

#include "Serializable.h"
#include "../converters/Serializer.h"

class Header: public Serializable{
    std::string name;
    std::string value;

public:
    Header(){};
    Header(std::string name, std::string value): name(name), value(value){};

    std::string getName(){ return name; }
    std::string getValue(){ return value; }

    void writeAsString(boost::property_tree::ptree& pt) override {
        pt.put("name", this->name);
        pt.put("value", this->value);
    }

    void readAsString(boost::property_tree::ptree& pt) override {
        this->name = pt.get<std::string>("name");
        this->value = pt.get<std::string>("value");
    }

    std::string to_string() override {
        auto string = Serializer::serialize(*this);
        return std::string(string.begin(), string.end());
    }
};


#endif //REMOTE_BACKUP_HEADER_H
