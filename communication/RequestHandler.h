//
// Created by alessandro on 02/11/20.
//

#ifndef REMOTE_BACKUP_REQUESTHANDLER_H
#define REMOTE_BACKUP_REQUESTHANDLER_H

#include <string>
#include <boost/noncopyable.hpp>
#include "../models/Request.h"
#include "../models/Response.h"
#include <mutex>
#include <condition_variable>

class RequestHandler: private boost::noncopyable
{
public:
    explicit RequestHandler();

    void handleRequest(Request& request, Response& response);

private:
    std::string outputDirPath;

    void transferDirectory(Request& request, Response& response);

    void transferFile(Request& request, Response& response);

    bool authenticateClient(Request& request, Response& response);

    void checksumFile(Request& request, Response& response);

    void deleteResource(Request& request, Response& response);

    void createDirectories(const std::string& relativePath);
};


#endif //REMOTE_BACKUP_REQUESTHANDLER_H
