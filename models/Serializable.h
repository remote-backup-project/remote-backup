#ifndef REMOTE_BACKUP_SERIALIZABLE_H
#define REMOTE_BACKUP_SERIALIZABLE_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

class Serializable{
public:
    virtual void writeAsString(boost::property_tree::ptree& pt);
    virtual void readAsString(boost::property_tree::ptree& pt);

    virtual void append(std::string string);
    virtual void clear();
    virtual std::string get();

    virtual std::string to_string() = 0;
};

#endif //REMOTE_BACKUP_SERIALIZABLE_H
