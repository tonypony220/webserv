cmake_minimum_required(VERSION 3.17)
project(webserv)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++98")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98")
set(CMAKE_CXX_STANDARD 98)
#set(CMAKE_CXX_STANDARD 14)
#add_definitions(-std=c++98)
include_directories(.)

add_executable(webserv
		poll.cpp 
		Request.cpp
		Response.cpp
		Utils.cpp
		Server.cpp
		SocketTCP.cpp
		ParseConf.cpp
        )

set_property(TARGET webserv PROPERTY CXX_STANDARD 98)
