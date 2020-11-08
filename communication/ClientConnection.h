//
// Created by alessandro on 06/11/20.
//

#ifndef REMOTE_BACKUP_CLIENTCONNECTION_H
#define REMOTE_BACKUP_CLIENTCONNECTION_H


#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/array.hpp>
#include "../models/Request.h"
#include "../models/Response.h"
#include "../models/FileChunk.h"
#include "../converters/Parser.h"

using boost::asio::ip::tcp;
namespace asio = boost::asio;


class ClientConnection
        : public boost::enable_shared_from_this<ClientConnection>,
          private boost::noncopyable
{
public:
    explicit ClientConnection(boost::asio::io_context& ioContext,
                              std::string& server,
                              std::string& port);


    void start();

    void setRequest(Request request);

private:

    boost::asio::io_context::strand strand;

    boost::asio::ip::tcp::socket socket;

    tcp::resolver resolver;

    boost::asio::streambuf buffer;

    boost::array<char, 8192> array;

    tcp::resolver::results_type endpoints;

    tcp::resolver::results_type::iterator endpoint;

    Request request;

    Response response;

    FileChunk fileChunk;

    std::string hostname;

    std::string port;

    Parser parser;

    void handleResolve(const boost::system::error_code& err);

    void handleConnect(const boost::system::error_code& err);

    void handleWrite(const boost::system::error_code& err);

    void handleRead(const boost::system::error_code& err, std::size_t bytes_transferred);
};

typedef boost::shared_ptr<ClientConnection> ClientConnectionPtr;


#endif //REMOTE_BACKUP_CLIENTCONNECTION_H
