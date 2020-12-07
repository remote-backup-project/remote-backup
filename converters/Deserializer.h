#ifndef REMOTE_BACKUP_DESERIALIZER_H
#define REMOTE_BACKUP_DESERIALIZER_H

#include <vector>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>
#include "../models/Serializable.h"
#include "../utils/StringUtils.h"
#include "../utils/Logger.h"

class Deserializer {
public:

    /**
     * Deserialize custom ->
     *  prevede che il tipo T abbia un metodo readAsString(boost::property_tree::ptree)
     */
    template <typename T, typename std::enable_if<std::is_base_of<Serializable, T>::value>::type* = nullptr>
    static T deserialize(std::vector<char>& v){
        boost::property_tree::ptree pt;
        std::stringstream buf(std::string(v.begin(), v.end()));
        boost::property_tree::read_json(buf, pt);

        T obj;
        obj.readAsString(pt);
        return obj;
    }

    template <typename T, typename std::enable_if<std::is_base_of<Serializable, T>::value>::type* = nullptr>
    static T deserialize(std::string string){
        std::vector<char> vector(string.begin(), string.end());
        return deserialize<T>(vector);
    }

    template<typename T, typename std::enable_if<std::is_base_of<Serializable, T>::value>::type* = nullptr>
    static std::vector<T> deserializeVector(std::string& string){
        std::vector<T> vector;
        if(!string.empty() &&
            boost::algorithm::starts_with(string, "[") &&
            boost::algorithm::ends_with(string, "]") &&
            !boost::algorithm::equals(string, "[]"))
        {
            std::string temp = string.substr(1, string.size()-2);
            std::vector<std::string> splitted = StringUtils::split(temp, "\r\n");
            for(std::string s: splitted){
                if(!s.empty()){
                    std::vector<char> charVector(s.begin(), s.end());
                    auto deserialized = deserialize<T>(charVector);
                    vector.push_back(deserialized);
                }
            }
        }
        return vector;
    }
};

#endif //REMOTE_BACKUP_DESERIALIZER_H
