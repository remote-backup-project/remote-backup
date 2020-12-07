#include "../converters/Serializer.h"

void Serializable::writeAsString(boost::property_tree::ptree& pt){}

void Serializable::readAsString(boost::property_tree::ptree& pt){}

void Serializable::append(std::string string){}

void Serializable::clear(){}

std::string Serializable::get(){ return ""; }