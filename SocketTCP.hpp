#pragma once
#include <chrono>
#include <iostream>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <vector>
#include "Utils.hpp"
#include <netinet/in.h>
#include <fcntl.h>
#define LISTEN_QLEN 32

// member vector of objects same class
//class SocketTCP;

class SocketTCP {
	private:
		int 		port;
		int 		fd; // socket descriptor
		std::string buff;

	public:
		SocketTCP(int port = 80);
		~SocketTCP();
		/* it's should not be copyable but it's better than use pointers */
		SocketTCP( const SocketTCP & copy );
		SocketTCP & operator=( SocketTCP const & other );
		int openSocket();

		int closeSocket() ;
		int 		getFd()   const ;
		std::string getBuff() const ;
		int		    getPort() const ;

};

std::ostream & operator<<( std::ostream & o, SocketTCP & s ) ;
