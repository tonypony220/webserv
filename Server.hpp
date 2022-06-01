#pragma once
#include "tcpSession.hpp"
#include "SocketTCP.hpp"

class Server {
public: 
	std::vector<SocketTCP> sockets;
	std::string root;
	std::string cgi_extension;
	bool dir_listing;

	Server( std::vector<int> & ports, std::string root_path ) : root(root_path) {
		for (int i = 0; i < ports.size(); i++)
			sockets.push_back(SocketTCP(ports[i]));
	}
	~Server() {}
	//Server( const Server & copy ) { *this = copy; }
	//Server & operator=( const Server & other ) {}

};
