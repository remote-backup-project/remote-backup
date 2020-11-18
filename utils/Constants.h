//
// Created by alessandro on 21/10/20.
//

#ifndef REMOTE_BACKUP_CONSTANTS_H
#define REMOTE_BACKUP_CONSTANTS_H

#include <string>

namespace Socket{
    const int CHUNK_SIZE = 5000;
};

namespace Config{
    const char USERNAME[] = "username";
    const char INPUT_DIR_PATH[] = "inputDirPath";
    const char OUTPUT_DIR_PATH[] = "outputDirPath";
    const char HOSTNAME[] = "hostname";
    const char PORT[] = "port";
};

namespace Services{
    const char TRANSFER_FILE[] = "/transfer/file";
    const char TRANSFER_DIRECTORY[] = "/transfer/directory";
    const char CHECKSUM_FILE[] = "/checksum";
    const char DELETE_RESOURCE[]= "/deleteResource";
}

namespace StockResponse {

    const char CONTINUE[] = "CONTINUE";
    const char OK[] = "OK";
    const char BAD_REQUEST[] = "BAD REQUEST";
    const char UNAUTHORIZED[] = "UNAUTHORIZED";
    const char FORBIDDEN[] = "FORBIDDEN";
    const char NOT_FOUND[] = "NOT FOUND";
    const char INTERNAL_SERVER_ERROR[] = "INTERNAL SERVER ERROR";

    enum StatusType
    {
        continue_ = 100,
        ok = 200,
        bad_request = 400,
        unauthorized = 401,
        forbidden = 403,
        not_found = 404,
        internal_server_error = 500,
    };
}

namespace FileWatcherStatus {
    enum class FileStatus {
        CREATED,
        MODIFIED,
        ERASED
    };
}

#endif //REMOTE_BACKUP_CONSTANTS_H
