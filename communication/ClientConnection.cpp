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
#include "../models/FileConfig.h"
#include "../utils/Constants.h"

using boost::asio::ip::tcp;
namespace asio = boost::asio;

ClientConnection::ClientConnection(boost::asio::io_context& ioContext):
   strand(ioContext),
   socket(ioContext),
   resolver(ioContext)
{}

boost::asio::ip::tcp::socket& ClientConnection::getSocket()
{
    return socket;
}

void ClientConnection::setRequest(Request request)
{
    this->request = request;
    this->request.addHeader(Header(Config::USERNAME, fileConfig.getUsername()));
    std::string inputDirPath(fileConfig.getInputDirPath());
    std::replace(inputDirPath.begin(), inputDirPath.end(), '/', '_');
    this->request.addHeader(Header(Config::INPUT_DIR_PATH, inputDirPath));
    this->fileChunk = Deserializer::deserialize<FileChunk>(request.getBody()); // serve solo per i log
}

void ClientConnection::setCallback(void* client, void (* callback)(void*, std::string))
{
    this->context = client;
    this->onResponse = callback;
}

void ClientConnection::start()
{
    LOG.debug("ClientConnection::start - File/Directory = " + fileChunk.getRelativePath() + ", Chuck = " + std::to_string(fileChunk.getChunkNumber()));
    boost::system::error_code error;
    this->endpoints = resolver.resolve(asio::ip::tcp::resolver::query(fileConfig.getHostname(), fileConfig.getPort()), error);
    handleResolve(error);
}

void ClientConnection::handleResolve(const boost::system::error_code& err)
{
    if (!err)
    {
        if(fileChunk.isDirectory())
            LOG.debug("ClientConnection::handleResolve - Directory = " + fileChunk.getRelativePath());
        else
            LOG.debug("ClientConnection::handleResolve - File = " + fileChunk.getRelativePath() + ", Chuck = " + std::to_string(fileChunk.getChunkNumber()));
        boost::system::error_code error;
        if(endpoint == endpoints.end())
            endpoint = endpoints.begin();
        socket.connect(endpoint->endpoint(), error);
        handleConnect(error);
    }
    else
    {
        LOG.warning("ClientConnection::handleResolve - < message : " + err.message() + " > - Retrying in 5 seconds");
        restart();
    }
}

void ClientConnection::handleConnect(const boost::system::error_code& err)
{
    if (!err)
    {
        if(fileChunk.isDirectory())
            LOG.debug("ClientConnection::handleConnect - Directory = " + fileChunk.getRelativePath());
        else
            LOG.debug("ClientConnection::handleConnect - File = " + fileChunk.getRelativePath() + ", Chuck = " + std::to_string(fileChunk.getChunkNumber()));
        StringUtils::fillStreambuf(buffer, request.to_string());
        boost::asio::async_write(socket, buffer,
                                 strand.wrap(boost::bind(&ClientConnection::handleWrite,
                                                     shared_from_this(),
                                                     boost::asio::placeholders::error)));
    }
    else
    {
        LOG.warning("ClientConnection::handleConnect - < message : " + err.message() + " > - Retrying in 5 seconds");
        restart();
    }
}

void ClientConnection::handleWrite(const boost::system::error_code& err)
{
    if(!socket.is_open())
        return;

    if (!err)
    {
        if(fileChunk.isDirectory())
            LOG.debug("ClientConnection::handleWrite - Directory = " + fileChunk.getRelativePath());
        else
            LOG.debug("ClientConnection::handleWrite - File = " + fileChunk.getRelativePath() + ", Chuck = " + std::to_string(fileChunk.getChunkNumber()));
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
        restart();
    }
}

void ClientConnection::handleRead(const boost::system::error_code& e, std::size_t bytes_transferred)
{
    if(!socket.is_open())
        return;

    if (!e)
    {
        if(fileChunk.isDirectory())
            LOG.debug("ClientConnection::handleRead - Directory = " + fileChunk.getRelativePath());
        else
            LOG.debug("ClientConnection::handleRead - File = " + fileChunk.getRelativePath() + ", Chuck = " + std::to_string(fileChunk.getChunkNumber()));
        boost::tribool result;
        boost::tie(result, boost::tuples::ignore) = parser.parse<Response>(response, std::string(array.data(), bytes_transferred));

        if(result)
        {
            handleResponse();
        }
        else if (!result)
        {
            LOG.warning("ClientConnection::handleRead - Cannot read Response = " + std::string(array.data(), bytes_transferred));
            LOG.warning("ClientConnection::handleRead - Retrying in 5 seconds");
            restart();
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
        LOG.error("ClientConnection::handleRead - Unexpected error < " + e.message() + " >");
    }
}

void ClientConnection::restart()
{
    socket.close();
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    start();
}

void ClientConnection::handleResponse()
{
    switch (response.status)
    {
        case StockResponse::continue_:
            LOG.trace("ClientConnection::handleResponse - Response < " + std::to_string(response.status) + " : " + response.getContent() + " >");
            socket.close();
            onResponse(context, fileChunk.getPath());
            break;
        case StockResponse::ok:
            LOG.info("ClientConnection::handleResponse - Response < " + std::to_string(response.status) + " : " + response.getContent() + " >");
            break;
        case StockResponse::not_found:
        case StockResponse::internal_server_error:
        case StockResponse::unauthorized:
            LOG.error("ClientConnection::handleResponse - Response < " + std::to_string(response.status) + " : " + response.getContent() + " >");
            LOG.warning("ClientConnection::handleResponse - Retrying in 5 seconds");
            restart();
            break;
        default:
            LOG.warning("ClientConnection::handleResponse - Unexpected Response < " + std::to_string(response.status) + " : " + response.getContent() + " >");
    }
}
