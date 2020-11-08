//
// Created by alessandro on 02/11/20.
//

#ifndef REMOTE_BACKUP_RESPONSE_H
#define REMOTE_BACKUP_RESPONSE_H

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include "Header.h"
#include "../converters/Serializer.h"
#include "../converters/Deserializer.h"

class Response : public Serializable
{
    std::vector<Header> headers;

    std::string content;

    std::string responseString;

public:
    enum StatusType
    {
        ok = 200,
        created = 201,
        accepted = 202,
        no_content = 204,
        multiple_choices = 300,
        moved_permanently = 301,
        moved_temporarily = 302,
        not_modified = 304,
        bad_request = 400,
        unauthorized = 401,
        forbidden = 403,
        not_found = 404,
        internal_server_error = 500,
        not_implemented = 501,
        bad_gateway = 502,
        service_unavailable = 503
    } status;

    static Response stockResponse(StatusType status);

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
