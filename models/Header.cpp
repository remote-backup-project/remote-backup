//
// Created by alessandro on 08/11/20.
//

#include "Header.h"

Header::Header(){}

Header::Header(std::string name, std::string value): name(name), value(value){}

std::string Header::getName(){ return name; }

std::string Header::getValue(){ return value; }

void Header::writeAsString(boost::property_tree::ptree& pt) {
    pt.put("name", this->name);
    pt.put("value", this->value);
}

void Header::readAsString(boost::property_tree::ptree& pt) {
    this->name = pt.get<std::string>("name");
    this->value = pt.get<std::string>("value");
}

std::string Header::to_string() {
    auto string = Serializer::serialize(*this);
    return std::string(string.begin(), string.end());
}