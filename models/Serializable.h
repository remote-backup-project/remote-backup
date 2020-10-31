//
// Created by alessandro on 31/10/20.
//

#ifndef REMOTE_BACKUP_SERIALIZABLE_H
#define REMOTE_BACKUP_SERIALIZABLE_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

class Serializable{
public:
    virtual void writeAsString(boost::property_tree::ptree& pt) = 0;
    virtual void readAsString(boost::property_tree::ptree& pt) = 0;
};

#endif //REMOTE_BACKUP_SERIALIZABLE_H
