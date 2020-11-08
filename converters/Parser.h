//
// Created by alessandro on 02/11/20.
//

#ifndef REMOTE_BACKUP_PARSER_H
#define REMOTE_BACKUP_PARSER_H

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>
#include "../models/Serializable.h"
#include "Deserializer.h"
#include <iostream>

class Parser{
public:
    Parser() {}

    /**
     * @param obj - da riempire
     * @param string - stringa (anche incompleta) da parsificare
     * @return tribool:
     *      true => request completa
     *      false => request non valida (caso in cui ci sia un'eccezione diversa da json_parser_exception)
     *      indeterminate => request non completa
     */
    template <typename T, typename std::enable_if<std::is_base_of<Serializable, T>::value>::type* = nullptr>
    boost::tuple<boost::tribool, std::string> parse(T& obj, std::string string)
    {
        obj.append(string);
        boost::tribool result = false;

        try
        {
            obj = Deserializer::deserialize<T>(obj.get());
            obj.clear();
            result = true;
        }
        catch(boost::property_tree::json_parser_error& exception)
        {
            result = boost::indeterminate;
        }

        return boost::make_tuple(result, string);
    }

};



#endif //REMOTE_BACKUP_PARSER_H
