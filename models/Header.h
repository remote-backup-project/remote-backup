#ifndef REMOTE_BACKUP_HEADER_H
#define REMOTE_BACKUP_HEADER_H

#include "Serializable.h"
#include "../converters/Serializer.h"

class Header: public Serializable{

    std::string name;

    std::string value;

public:
    Header();

    Header(std::string name, std::string value);

    std::string getName();

    std::string getValue();

    void writeAsString(boost::property_tree::ptree& pt) override;

    void readAsString(boost::property_tree::ptree& pt) override;

    std::string to_string() override;
};


#endif //REMOTE_BACKUP_HEADER_H
