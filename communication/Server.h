#ifndef REMOTE_BACKUP_SERVER_H
#define REMOTE_BACKUP_SERVER_H

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "ServerConnection.h"

class Server : private boost::noncopyable {
public:
    explicit Server(std::size_t thread_pool_size);

    void run();

private:
    std::size_t threadPoolSize;

    boost::asio::io_context ioContext;

    boost::asio::signal_set signals;

    boost::asio::ip::tcp::acceptor acceptor;

    ConnectionPtr newConnection;

    RequestHandler requestHandler;

    void startAccept();

    void handleAccept(const boost::system::error_code& e);

    void handleStop();
};


#endif //REMOTE_BACKUP_SERVER_H
