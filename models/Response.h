#ifndef REMOTE_BACKUP_RESPONSE_H
#define REMOTE_BACKUP_RESPONSE_H

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include "Header.h"
#include "../converters/Serializer.h"
#include "../converters/Deserializer.h"
#include "../utils/Constants.h"

class Response : public Serializable
{
    std::vector<Header> headers;

    std::string content;

    std::string responseString;

public:

    StockResponse::StatusType status;

    static Response stockResponse(StockResponse::StatusType status);

    std::vector<Header> getHeaders();

    void addHeader(Header& header);

    std::string getContent();

    void appendContent(const char* buf, size_t size);

    std::string getResponseString();

    void writeAsString(boost::property_tree::ptree& pt) override;

    void readAsString(boost::property_tree::ptree& pt) override;

    void append(std::string string) override;

    std::string get() override;

    void clear() override;

    std::string to_string() override;
};

#endif //REMOTE_BACKUP_RESPONSE_H
