#ifndef REMOTE_BACKUP_SERVERCONNECTION_H
#define REMOTE_BACKUP_SERVERCONNECTION_H

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "../models/Response.h"
#include "../models/Request.h"
#include "RequestHandler.h"
#include "../converters/Parser.h"

using boost::asio::ip::tcp;
namespace asio = boost::asio;

class ServerConnection
        : public boost::enable_shared_from_this<ServerConnection>,
          private boost::noncopyable
{
public:
    explicit ServerConnection(boost::asio::io_context& ioContext);

    boost::asio::ip::tcp::socket& getSocket();

    void start();

private:
    void handleRead(const boost::system::error_code& e, std::size_t bytes_transferred);

    void handleWrite(const boost::system::error_code& e);

    boost::asio::io_context::strand strand;

    boost::asio::ip::tcp::socket socket;

    RequestHandler requestHandler;

    boost::array<char, 8192> array;

    boost::asio::streambuf buffer;

    Request request;

    Parser parser;

    Response response;
};

typedef boost::shared_ptr<ServerConnection> ConnectionPtr;

#endif //REMOTE_BACKUP_SERVERCONNECTION_H
