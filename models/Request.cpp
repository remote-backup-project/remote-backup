#include "Request.h"
#include <utility>
#include "../converters/Serializer.h"
#include "../converters/Deserializer.h"

Request::Request() {}

Request::Request(std::string uri): uri(std::move(uri)){}

Request::Request(std::string uri, std::vector<Header>& headers):
    uri(std::move(uri)), headers(headers){}

Request::Request(std::string uri, std::string body):
    uri(std::move(uri)), body(std::move(body)){}

Request::Request(std::string uri, std::vector<Header>& headers, std::string body):
    uri(std::move(uri)), headers(headers), body(std::move(body)){}


std::string Request::getUri(){ return uri; }

std::vector<Header>& Request::getHeaders(){ return headers; }

void Request::addHeader(Header header) { headers.push_back(header); }

std::string Request::getBody(){ return body; }

void Request::writeAsString(boost::property_tree::ptree& pt){
    pt.put("uri", this->uri);
    pt.put("body", this->body);
    pt.put("headers", Serializer::serializeVector(headers));
}

void Request::readAsString(boost::property_tree::ptree& pt){
    this->uri = pt.get<std::string>("uri");
    this->body = pt.get<std::string>("body");
    auto headersString = pt.get<std::string>("headers");
    this->headers = Deserializer::deserializeVector<Header>(headersString);
}

void Request::append(std::string string){ requestString.append(string); }

void Request::clear(){ requestString.clear(); }

std::string Request::get(){ return requestString; }

void Request::setBody(std::string body){ this->body = std::move(body); }

void Request::setUri(std::string uri){ this->uri = std::move(uri); }

std::string Request::to_string(){
    auto string = Serializer::serialize(*this);
    return std::string(string.begin(), string.end());
}