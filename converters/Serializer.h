//
// Created by alessandro on 21/10/20.
//

#ifndef REMOTE_BACKUP_SERIALIZER_H
#define REMOTE_BACKUP_SERIALIZER_H

#include <vector>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "../models/Serializable.h"

class Serializer {
public:

    /**
     * Serialize custom ->
     *  prevede che il tipo T abbia un metodo writeAsString(boost::property_tree::ptree)
     */
    template <typename T, typename std::enable_if<std::is_base_of<Serializable, T>::value>::type* = nullptr>
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

    template<typename T, typename std::enable_if<std::is_base_of<Serializable, T>::value>::type* = nullptr>
    static std::string serializeVector(std::vector<T> vector){
        std::string s("[");
        for(T el : vector){
            auto serialized = serialize(el);
            std::string serializedString(serialized.begin(), serialized.end());
            s.append(serializedString + "\r\n");
        }
        s.append("]");
        return s;
    }

};


#endif //REMOTE_BACKUP_SERIALIZER_H
