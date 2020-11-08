//
// Created by alessandro on 06/11/20.
//

#include "ClientConnection.h"
#include <iostream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include <boost/logic/tribool.hpp>
#include "../utils/Logger.h"
#include "../converters/Deserializer.h"

using boost::asio::ip::tcp;
namespace asio = boost::asio;

ClientConnection::ClientConnection(boost::asio::io_context& ioContext,
                                   std::string& hostname,
                                   std::string& port):
   strand(ioContext),
   socket(ioContext),
   resolver(ioContext),
   hostname(hostname),
   port(port)
{}

void ClientConnection::setRequest(Request request){
    this->request = request;
    this->fileChunk = Deserializer::deserialize<FileChunk>(request.getBody());
}

void ClientConnection::start()
{
    LOG.info("ClientConnection::start - File/Directory = " + fileChunk.getRelativePath() + ", Chuck = " + std::to_string(fileChunk.getChunkNumber()));
    boost::system::error_code error;
    this->endpoints = resolver.resolve(asio::ip::tcp::resolver::query(hostname, port), error);
    handleResolve(error);
}

void ClientConnection::handleResolve(const boost::system::error_code& err)
{
    if (!err)
    {
        LOG.info("ClientConnection::handleResolve - File/Directory = " + fileChunk.getRelativePath() + ", Chuck = " + std::to_string(fileChunk.getChunkNumber()));
        boost::system::error_code error;
        if(endpoint == endpoints.end())
            endpoint = endpoints.begin();
        socket.connect(endpoint->endpoint(), error);
        handleConnect(error);
    }
    else
    {
        LOG.warning("ClientConnection::handleResolve - < message : " + err.message() + " > - Retrying in 5 seconds");
        boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
        start();
    }
}

void ClientConnection::handleConnect(const boost::system::error_code& err)
{
    if (!err)
    {
        LOG.info("ClientConnection::handleConnect - File/Directory = " + fileChunk.getRelativePath() + ", Chuck = " + std::to_string(fileChunk.getChunkNumber()));
        StringUtils::fillStreambuf(buffer, request.to_string());
        boost::asio::async_write(socket, buffer,
                                 strand.wrap(boost::bind(&ClientConnection::handleWrite,
                                                     shared_from_this(),
                                                     boost::asio::placeholders::error)));
    }
    else
    {
        LOG.warning("ClientConnection::handleConnect - < message : " + err.message() + " > - Retrying in 5 seconds");
        socket.close();
        boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

        if(endpoint == endpoints.end())
            start();
        else
            handleResolve(boost::system::error_code());
    }
}

void ClientConnection::handleWrite(const boost::system::error_code& err)
{
    if(!socket.is_open())
        return;

    if (!err)
    {
        LOG.info("ClientConnection::handleWrite - File/Directory = " + fileChunk.getRelativePath() + ", Chuck = " + std::to_string(fileChunk.getChunkNumber()));
        socket.async_read_some(boost::asio::buffer(array),
                               strand.wrap(
                                       boost::bind(&ClientConnection::handleRead,
                                                   shared_from_this(),
                                                   boost::asio::placeholders::error,
                                                   boost::asio::placeholders::bytes_transferred)));
    }
    else
    {
        LOG.error("ClientConnection::handleWrite - < message :  " + err.message() + " > - Retrying in 5 seconds");
        socket.close();
        boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
        start();
    }
}

void ClientConnection::handleRead(const boost::system::error_code& e, std::size_t bytes_transferred)
{
    if(!socket.is_open())
        return;

    if (!e)
    {
        LOG.info("ClientConnection::handleRead - File/Directory = " + fileChunk.getRelativePath() + ", Chuck = " + std::to_string(fileChunk.getChunkNumber()));
        boost::tribool result;
        boost::tie(result, boost::tuples::ignore) = parser.parse<Response>(response, std::string(array.data(), bytes_transferred));

        if(result)
        {
            switch (response.status) {
                case Response::ok:
                    LOG.info("ClientConnection::handleRead - Response < " + std::to_string(response.status) + " : " + response.getContent() + " >");
                    break;
                case Response::not_found:
                case Response::internal_server_error: //TODO aggiungere caso forbidden o unauthorized quando aggiungi login
                    LOG.error("ClientConnection::handleRead - Response < " + std::to_string(response.status) + " : " + response.getContent() + " >");
                    LOG.warning("ClientConnection::handleRead - Retrying in 5 seconds");
                    socket.close();
                    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
                    start();
                    break;
                default:
                    LOG.warning("ClientConnection::handleRead - Unexpected Response < " + std::to_string(response.status) + " : " + response.getContent() + " >");
            }
        }
        else if (!result)
        {
            LOG.warning("ClientConnection::handleRead - Cannot read Response = " + std::string(array.data(), bytes_transferred));
            LOG.warning("ClientConnection::handleRead - Retrying in 5 seconds");
            socket.close();
            boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
            start();
        }
        else
        {
            LOG.warning("ClientConnection::handleRead - Response not finished yet => Reading again");
            socket.async_read_some(boost::asio::buffer(array),
                                   strand.wrap(
                                           boost::bind(&ClientConnection::handleRead,
                                                       shared_from_this(),
                                                       boost::asio::placeholders::error,
                                                       boost::asio::placeholders::bytes_transferred)));
        }
    }
    else
    {   //TODO da vedere se fare altro in caso di errore di lettura
        LOG.error("ClientConnection::handleRead - Unexpected error , < " + e.message() + " >");
    }
}