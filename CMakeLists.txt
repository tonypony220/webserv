cmake_minimum_required(VERSION 3.17)
project(webserv)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(webserv
        main.cpp
        one.hpp
        poll.cpp
        iter.hpp
        select.cpp
#        session.cpp
        Session.hpp
        SocketTCP.c
        SocketTCP.hpp
        t.cpp
        tcp_questn.c)
