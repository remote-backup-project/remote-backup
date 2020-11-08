//
// Created by alessandro on 02/11/20.
//

#include "Server.h"
#include "../utils/Logger.h"
#include <iostream>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include "../models/FileConfig.h"

Server::Server(std::size_t threadPoolSize)
        : threadPoolSize(threadPoolSize),
          signals(ioContext),
          acceptor(ioContext),
          newConnection()
{
    signals.add(SIGINT);
    signals.add(SIGTERM);
#if defined(SIGQUIT)
    signals.add(SIGQUIT);
#endif
    signals.async_wait(boost::bind(&Server::handleStop, this));

    fileConfig.readServerFile();

    boost::asio::ip::tcp::resolver resolver(ioContext);
    boost::asio::ip::tcp::resolver::query query(fileConfig.getHostname(), fileConfig.getPort());
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    acceptor.open(endpoint.protocol());
    acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen();

    startAccept();
}

void Server::run()
{
    // Creo pool di thread dove tutti runnano ioContext.
    std::vector<boost::shared_ptr<boost::thread> > threads;
    for (std::size_t i = 0; i < threadPoolSize; ++i)
    {
        boost::shared_ptr<boost::thread> thread(new boost::thread([ioContextPtr = &ioContext] { ioContextPtr->run(); }));
        threads.push_back(thread);
    }

    for (auto & thread : threads)
        thread->join();
}

void Server::startAccept()
{
    LOG.info("Server::startAccept");
    newConnection.reset(new ServerConnection(ioContext));
    acceptor.async_accept(newConnection->getSocket(),
                           boost::bind(&Server::handleAccept,
                                       this,
                                       boost::asio::placeholders::error));
}

void Server::handleAccept(const boost::system::error_code& e)
{
    LOG.info("Server::handleAccept");
    if (!e)
    {
        newConnection->start();
    }
    else
    {
        LOG.error("Server::handleAccept - unexpected error < " + e.message() + " >");
    }

    startAccept();
}

void Server::handleStop()
{
    ioContext.stop();
}