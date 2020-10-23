//
// Created by alessandro on 21/10/20.
//

#ifndef REMOTE_BACKUP_DESERIALIZER_H
#define REMOTE_BACKUP_DESERIALIZER_H

#include <vector>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

class Deserializer {
public:

    /**
     * Deserialize custom ->
     *  prevede che il tipo T abbia un metodo readAsString(boost::property_tree::ptree)
     */
    template <typename T>
    static T deserialize(std::vector<char>& v){
        boost::property_tree::ptree pt;
        std::stringstream buf(std::string(v.begin(), v.end()));
        boost::property_tree::read_json(buf, pt);

        T obj;
        obj.readAsString(pt);
        return obj;
    }
};

#endif //REMOTE_BACKUP_DESERIALIZER_H
