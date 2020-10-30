//
// Created by gaetano on 26/10/20.
//

#ifndef REMOTE_BACKUP_FILEEXCEPTION_H
#define REMOTE_BACKUP_FILEEXCEPTION_H

#include <iostream>


class FileException : std::exception {
    std::string message;
public:
    FileException(){}
    FileException(const std::string& msg) : message(msg){}
    std::string getMessage() { return message; }
    ~FileException(){}
};


#endif //REMOTE_BACKUP_FILEEXCEPTION_H
