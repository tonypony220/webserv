cmake_minimum_required(VERSION 3.17)
project(webserv)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++98")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98")
set(CMAKE_CXX_STANDARD 98)
#set(CMAKE_CXX_STANDARD 14)
#add_definitions(-std=c++98)
include_directories(.)

add_executable(webserv
        main.cpp
        one.hpp
        poll.cpp
        iter.hpp
        select.cpp
#        session.cpp
        HttpSession.hpp
        SocketTCP.c
        SocketTCP.hpp
        t.cpp
        tcp_questn.c)

set_property(TARGET webserv PROPERTY CXX_STANDARD 98)