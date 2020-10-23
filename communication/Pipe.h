//
// Created by alessandro on 23/10/20.
//

#ifndef REMOTE_BACKUP_PIPE_H
#define REMOTE_BACKUP_PIPE_H

#include <iostream>
#include "../converters/Deserializer.h"
#include "../converters/Serializer.h"

class Pipe{
    int channel[2];
    pid_t pid;
public:
    Pipe& openConnection(){ //TODO sostituire con openConnection(socket)
        if (pipe(this->channel) == -1){
            std::cout<<"Errore nella pipe per il channel"<<std::endl;
        }

        pid = fork();

        return *this;
    }

    bool hasErrors(){
        if (pid == Constants::Pipe::FORK_ERROR) {
            std::cout << "Errore nella fork" << std::endl;
            return true;
        }
        return false;
    }

    template <typename T>
    T read_pipe(){    //TODO sostituire con recv_socket
        ushort size = 0;
        read(channel[Constants::Pipe::READ], &size, sizeof(ushort));
        std::vector<char> buff(size);
        read(channel[Constants::Pipe::READ], reinterpret_cast<char*>(buff.data()), size);
        return Deserializer::deserialize<T>(buff);
    }

    template <typename T>
    void write_pipe(T obj){ //TODO sostituire con send_socket
        auto buff = Serializer::serialize<T>(obj);
        ushort size = buff.size();
        write(channel[Constants::Pipe::WRITE], &size, sizeof(ushort));
        write(channel[Constants::Pipe::WRITE], reinterpret_cast<char*>(buff.data()), size);
    }

    pid_t getPid(){ return pid; }

    void closeChildInRead(){ close(channel[Constants::Pipe::READ]); }
    void closeParentInWrite(){ close(channel[Constants::Pipe::WRITE]); }

};

#endif //REMOTE_BACKUP_PIPE_H
