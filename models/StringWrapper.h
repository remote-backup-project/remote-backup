//
// Created by gaetano on 25/10/20.
//

#ifndef REMOTE_BACKUP_STRINGWRAPPER_H
#define REMOTE_BACKUP_STRINGWRAPPER_H

#include <string>
#include "Serializable.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "../converters/Serializer.h"

//TODO da levare
class StringWrapper : public Serializable{
    std::string content;
public:
    StringWrapper() {}
    StringWrapper(std::string content) : content(content){}

    std::string getContent() { return content; }

    void writeAsString(boost::property_tree::ptree& pt) override{
        pt.put("content", this->content);
    }
    void readAsString(boost::property_tree::ptree& pt) override{
        this->content = pt.get<std::string>("content");
    }

    std::string to_string(){
        auto string = Serializer::serialize(*this);
        return std::string(string.begin(), string.end());
    }
};

#endif //REMOTE_BACKUP_STRINGWRAPPER_H
