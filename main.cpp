#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <csignal>
#include "converters/Serializer.h"
#include "converters/Deserializer.h"
#include "utils/Constants.h"
#include "FileInfo.h"
#include <sys/stat.h>

namespace fs = std::filesystem;

template <typename T>
T read_pipe(int fd){
    ushort size = 0;
    read(fd, &size, sizeof(ushort));
    std::vector<char> buff(size);
    read(fd, reinterpret_cast<char*>(buff.data()), size);
    return Deserializer::deserialize<T>(buff);
}

template <typename T>
void write_pipe(int fd, T obj){
    auto buff = Serializer::serialize<T>(obj);
    ushort size = buff.size();
    write(fd, &size, sizeof(ushort));
    write(fd, reinterpret_cast<char*>(buff.data()), size);
}

pid_t openConnection(int* channel){
    if (pipe(channel) == -1){
        std::cout<<"Errore nella pipe per il channel"<<std::endl;
    }

    pid_t pid = fork();
    if (pid == Constants::Pipe::FORK_ERROR) {
        std::cout << "Errore nella fork" << std::endl;
    }

    return pid;
}

void sendFile(const std::string& filename, int channel)
{
    std::string v2;
    std::ifstream ifs;
    ifs.open(filename, std::ios::in | std::ios::binary);

    if(!ifs.is_open())
        throw std::exception();

    std::vector<char> data(Constants::Pipe::MAX_BYTE + 1,0);
    int i = 0;
    while (true)
    {
        i++;
        ifs.read(data.data(), Constants::Pipe::MAX_BYTE);
        std::streamsize s = ((ifs)? Constants::Pipe::MAX_BYTE : ifs.gcount());
        data[s] = 0;
        FileInfo fileInfo(std::string(data.data(), s), filename);
        write_pipe(channel, fileInfo);
        if(!ifs)
            break;
    }
    ifs.close();
}

void finishSending(int channel){
    write_pipe(channel, FileInfo());
}

void sendDirectory(const std::string& directory, int channel){
    fs::path path(directory);
    for(auto &p : fs::recursive_directory_iterator(path)){
        std::cout << p.path().string() << std::endl;
        if( !fs::is_directory(p.status()) ){
            sendFile(p.path().string(), channel);
        }
    }
    finishSending(channel);
}

void receiveDirectory(const std::string& outputDir, int channel){
    std::ofstream ofs3;
    while(true){
        auto fileInfo = read_pipe<FileInfo>(channel);

        if(!fileInfo.isValid())
            return;

//        if(fileInfo.isDirectory()){
//            const int dir_err = mkdir(std::string(outputDir + "/" + fileInfo.getFilename()).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
//            if (-1 == dir_err)
//            {
//                printf("Error creating directory!n");
//                exit(1);
//            }
//        }

        if(!ofs3.is_open()){
            ofs3 = std::ofstream(outputDir + "/" + fileInfo.getFilename(), std::ios::out | std::ios::binary);
            if(!ofs3.is_open()) throw std::exception();
        }

        ofs3.write(fileInfo.getContent().data(), fileInfo.getContent().size()*sizeof(char));
        if(fileInfo.getContent().size() < Constants::Pipe::MAX_BYTE)
            ofs3.close();
    }
}

int main() {
    int channel[2];

    pid_t pid = openConnection(channel);

    switch(pid){
        case Constants::Pipe::CHILD:
            close(channel[Constants::Pipe::READ]);
            sendDirectory("/home/alessandro/CLionProjects/remote-backup/inputDirectory", channel[Constants::Pipe::WRITE]);
            exit(1);
        default:
            close(channel[Constants::Pipe::WRITE]);
            receiveDirectory("/home/alessandro/CLionProjects/remote-backup/outputDirectory", channel[Constants::Pipe::READ]);
    }

    return 0;
}
