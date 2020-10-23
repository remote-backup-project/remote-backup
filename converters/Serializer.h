//
// Created by alessandro on 21/10/20.
//

#ifndef REMOTE_BACKUP_SERIALIZER_H
#define REMOTE_BACKUP_SERIALIZER_H

#include <vector>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

class Serializer {
public:

    /**
     * Serialize custom ->
     *  prevede che il tipo T abbia un metodo writeAsString(boost::property_tree::ptree)
     */
    template <typename T>
    static std::vector<char> serialize(T obj){
        boost::property_tree::ptree pt;
        obj.writeAsString(pt);

        std::ostringstream buf;
        boost::property_tree::write_json(buf, pt, false);
        std::vector<char> result;
        std::string s = buf.str();
        std::copy(s.begin(), s.end(), std::back_inserter(result));
        return result;
    }

};


#endif //REMOTE_BACKUP_SERIALIZER_H
