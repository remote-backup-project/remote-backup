#include "Response.h"

Response Response::stockResponse(StockResponse::StatusType status)
{
    Response response;
    response.status = status;
    response.content = StringUtils::response_to_string(status);
    return response;
}

std::string Response::to_string(){
    auto string = Serializer::serialize(*this);
    return std::string(string.begin(), string.end());
}

std::vector<Header> Response::getHeaders(){ return headers; }

void Response::addHeader(Header& header) { headers.push_back(header); }

std::string Response::getContent(){ return content; }

void Response::appendContent(const char* buf, size_t size) { content.append(buf, size); }

std::string Response::getResponseString(){ return responseString; }

void Response::writeAsString(boost::property_tree::ptree& pt)
{
    pt.put("content", this->content);
    pt.put("status", static_cast<int>(this->status));
    pt.put("headers", Serializer::serializeVector(headers));
}

void Response::readAsString(boost::property_tree::ptree& pt)
{
    this->content = pt.get<std::string>("content");
    this->status = static_cast<StockResponse::StatusType>(pt.get<int>("status"));
    auto headersString = pt.get<std::string>("headers");
    this->headers = Deserializer::deserializeVector<Header>(headersString);
}

void Response::append(std::string string) { responseString.append(string); }

std::string Response::get() { return responseString; }

void Response::clear() { responseString.clear(); }