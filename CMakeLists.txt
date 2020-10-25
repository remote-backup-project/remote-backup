cmake_minimum_required(VERSION 3.17)
project(remote_backup)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -lboost_serialization")

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
FIND_PACKAGE( Boost REQUIRED COMPONENTS system filesystem serialization log REQUIRED)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

link_directories(${Boost_LIBRARY_DIR})

add_executable(remote_backup main.cpp socket.cpp socket.h serverSocket.cpp serverSocket.h utils/Logger.h)

TARGET_LINK_LIBRARIES( remote_backup LINK_PUBLIC ${Boost_LIBRARIES} -lstdc++fs)

