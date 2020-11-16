cmake_minimum_required(VERSION 3.17)
project(remote_backup)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -lboost_serialization")

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
find_package(Boost CONFIG REQUIRED COMPONENTS system filesystem serialization log log_setup iostreams REQUIRED)
find_package(OpenSSL REQUIRED)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

link_directories(${Boost_LIBRARY_DIR})

add_executable(start_server start_server.cpp utils/Logger.cpp models/Serializable.cpp communication/ServerConnection.cpp models/FileChunk.cpp communication/Server.cpp utils/StringUtils.cpp communication/RequestHandler.cpp models/Request.cpp models/Response.cpp converters/Parser.h models/FileConfig.cpp models/Header.cpp models/FileWriter.cpp)
add_executable(start_client start_client.cpp utils/Logger.cpp models/Serializable.cpp communication/ClientConnection.cpp models/FileChunk.cpp communication/Client.cpp utils/StringUtils.cpp models/Request.cpp models/Response.cpp converters/Parser.h models/FileConfig.cpp models/Header.cpp)

TARGET_LINK_LIBRARIES( start_server LINK_PUBLIC ${Boost_LIBRARIES} -lstdc++fs OpenSSL::SSL)
TARGET_LINK_LIBRARIES( start_client LINK_PUBLIC ${Boost_LIBRARIES} -lstdc++fs OpenSSL::SSL)


