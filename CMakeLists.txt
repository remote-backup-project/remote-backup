cmake_minimum_required(VERSION 3.17)
project(remote_backup)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -lboost_serialization" )

find_package(Boost 1.55 REQUIRED COMPONENTS system filesystem serialization REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

link_directories(${Boost_LIBRARY_DIR})

add_executable(remote_backup main.cpp converters/Serializer.h converters/Deserializer.h utils/Constants.h models/FileInfo.h utils/StringUtils.h communication/Pipe.h)
target_link_libraries(remote_backup LINK_PUBLIC ${Boost_LIBRARIES}  -lstdc++fs)