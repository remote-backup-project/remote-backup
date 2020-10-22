//
// Created by alessandro on 21/10/20.
//

#ifndef REMOTE_BACKUP_CONSTANTS_H
#define REMOTE_BACKUP_CONSTANTS_H

class Constants{
public:
    class Pipe {
    public:
        static const int FORK_ERROR = -1;
        static const int CHILD = 0;
        static const int READ = 0;
        static const int WRITE = 1;
        static const int MAX_BYTE = 1024;
    };
    class Socket{

    };
};

#endif //REMOTE_BACKUP_CONSTANTS_H
