//
// Created by alessandro on 22/10/20.
//

#ifndef REMOTE_BACKUP_STRINGUTILS_H
#define REMOTE_BACKUP_STRINGUTILS_H


#include <boost/asio.hpp>
#include <string>
#include <vector>
#include "Constants.h"

class StringUtils{
public:

    static std::string encodeBase64(const std::string& data);

    static std::string decodeBase64(const std::string& input, std::string& out);

    /**
     * @param s1
     * @param s2
     * @return s1 - s2
     */
    static std::string getStringDifference(const std::string& s1, const std::string& s2);

    static std::vector<std::string> split(const std::string& s, const std::string& delimitator);

    /**
     * @param streambuf
     * @param string
     *      string -> streambuf
     */
    static void fillStreambuf(boost::asio::streambuf& streambuf, std::string string);

    /**
     * @param streambuf
     * @return  streambuf -> string (dopo questa operazione streambuf è vuoto)
     */
    static std::string fillFromStreambuf(boost::asio::streambuf& streambuf);

    static std::string response_to_string(StockResponse::StatusType status);
};

#endif //REMOTE_BACKUP_STRINGUTILS_H
