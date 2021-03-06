#ifndef REMOTE_BACKUP_REQUEST_H
#define REMOTE_BACKUP_REQUEST_H

#include <string>
#include <utility>
#include <vector>
#include "Header.h"
#include "Serializable.h"
#include <boost/asio.hpp>

class Request: public Serializable{
    std::string uri;
    std::vector<Header> headers;
    std::string body;
    std::string requestString;

public:
    Request();
    Request(std::string uri);
    Request(std::string uri, std::vector<Header>& headers);
    Request(std::string uri, std::string body);
    Request(std::string uri, std::vector<Header>& headers, std::string body);

    std::string getUri();
    std::vector<Header>& getHeaders();
    void addHeader(Header header);
    std::string getBody();

    void setBody(std::string body);
    void setUri(std::string uri);

    void writeAsString(boost::property_tree::ptree& pt) override;

    void readAsString(boost::property_tree::ptree& pt) override;

    void append(std::string string) override;

    void clear() override;

    std::string get() override;

    std::string to_string() override;
};

#endif //REMOTE_BACKUP_REQUEST_H
