cmake_minimum_required(VERSION 3.17)
project(remote_backup)

set(CMAKE_CXX_STANDARD 17)

add_executable(remote_backup main.cpp socket.cpp socket.h serverSocket.cpp serverSocket.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

FIND_PACKAGE( Boost 1.70 COMPONENTS REQUIRED date_time)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

ADD_EXECUTABLE( exe main.cpp )

target_include_directories(exe PRIVATE ${Boost_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES( exe LINK_PUBLIC ${Boost_LIBRARIES} )

install(TARGETS exe RUNTIME DESTINATION bin)