//
// Created by alessandro on 02/11/20.
//

#ifndef REMOTE_BACKUP_REQUESTHANDLER_H
#define REMOTE_BACKUP_REQUESTHANDLER_H

#include <string>
#include <boost/noncopyable.hpp>
#include "../models/Request.h"
#include "../models/Response.h"

class RequestHandler: private boost::noncopyable
{
public:
    explicit RequestHandler();

    void handleRequest(Request& request, Response& response);

private:

    void transferDirectory(Request& request, Response& response);

    void transferFile(Request& request, Response& response);
};


#endif //REMOTE_BACKUP_REQUESTHANDLER_H
