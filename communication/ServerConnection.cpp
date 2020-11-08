//
// Created by alessandro on 02/11/20.
//

#include "ServerConnection.h"
#include <boost/bind.hpp>
#include <iostream>
#include <boost/thread/thread.hpp>
#include "../utils/Logger.h"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/chrono.hpp>


ServerConnection::ServerConnection(boost::asio::io_context& ioContext)
        : strand(ioContext),
          socket(ioContext)
{
}

boost::asio::ip::tcp::socket& ServerConnection::getSocket()
{
    return socket;
}

void ServerConnection::start()
{
    LOG.info("ServerConnection::start");
    socket.async_read_some(boost::asio::buffer(array),
                            strand.wrap(
                                    boost::bind(&ServerConnection::handleRead,
                                                shared_from_this(),
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred)));
}

void ServerConnection::handleRead(const boost::system::error_code& e, std::size_t bytes_transferred)
{
    LOG.info("ServerConnection::handleRead");
    if (!e)
    {
        boost::tribool result;
        boost::tie(result, boost::tuples::ignore) = parser.parse<Request>(request, std::string(array.data(), bytes_transferred));
        if (result)
        {
            requestHandler.handleRequest(request, response);
            StringUtils::fillStreambuf(buffer, response.to_string());
            boost::asio::async_write(socket, buffer,
                                     strand.wrap(
                                             boost::bind(&ServerConnection::handleWrite,
                                                         shared_from_this(),
                                                         boost::asio::placeholders::error)));
        }
        else if (!result)
        {
            response = Response::stockResponse(Response::bad_request);
            LOG.error("ServerConnection::handleRead - Response = < " + std::to_string(response.status) + " - Wrong request format  >");
            StringUtils::fillStreambuf(buffer, response.to_string());
            boost::asio::async_write(socket, buffer,
                                     strand.wrap(
                                             boost::bind(&ServerConnection::handleWrite,
                                                         shared_from_this(),
                                                         boost::asio::placeholders::error)));
        }
        else
        {
            LOG.warning("ServerConnection::handleRead - Request not finished yet => Reading again");
            socket.async_read_some(boost::asio::buffer(array),
                                    strand.wrap(
                                            boost::bind(&ServerConnection::handleRead,
                                                        shared_from_this(),
                                                        boost::asio::placeholders::error,
                                                        boost::asio::placeholders::bytes_transferred)));
        }
    }
    else
    {   //TODO da vedere se fare altro in caso di errore di lettura
        response = Response::stockResponse(Response::internal_server_error);
        LOG.error("ServerConnection::handleRead - Response = < " + std::to_string(response.status) + " - " + e.message() + "  >");
        StringUtils::fillStreambuf(buffer, response.to_string());
        boost::asio::async_write(socket, buffer,
                                 strand.wrap(
                                         boost::bind(&ServerConnection::handleWrite,
                                                     shared_from_this(),
                                                     boost::asio::placeholders::error)));
    }
}

void ServerConnection::handleWrite(const boost::system::error_code& e)
{
    LOG.info("ServerConnection::handleWrite");
    if (!e)
    {
        boost::system::error_code ignored_ec;
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    }
    else
    {
        LOG.error("ServerConnection::handleWrite - Error < " + e.message() + " >");
    }
}

