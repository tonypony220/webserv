#pragma once
#include "Session.hpp"
#include "SocketTCP.hpp"
#include <algorithm>
#include <map>

//std::map<std::string, std::string> execs = {std::make_pair(".py", "python3")};

class Server {
public: 
	std::vector<SocketTCP> sockets;
	std::string root;
	std::vector<std::string> cgi_extensions;
	bool dir_listing;

	std::map<std::string, std::string> execs;

	Server( std::vector<int> & ports, std::string root_path )
	: root(root_path) {
		for (int i = 0; i < ports.size(); i++)
			sockets.push_back(SocketTCP(ports[i]));
//		execs[".py"] = "python3";
//		execs[".cgi"] = "./";
//		execs.insert(std::make_pair(".py", "python3"));
	}
	~Server() {}
	//Server( const Server & copy ) { *this = copy; }
	//Server & operator=( const Server & other ) {}
	void enable_cgi(const std::string & file_extension) {
		cgi_extensions.push_back(file_extension);
	}

	bool is_cgi_file_type(std::string & file_type) {
		log(file_type, " cgi extensions:  ");
		p(cgi_extensions);
		std::vector<std::string>::iterator found = find(cgi_extensions.begin(),
			 											cgi_extensions.end(),
			 											file_type);
		if ( found != cgi_extensions.end())
			return true;
		return false;

	}
};
