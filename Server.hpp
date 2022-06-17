#pragma once
#include "Session.hpp"
#include "SocketTCP.hpp"
#include <algorithm>
#include <map>
#include <set>

//std::map<std::string, std::string> execs = {std::make_pair(".py", "python3")};
struct Location {
	std::string 			 root;
	std::string 		     redirect_uri;
	bool 					 dir_listing;
	std::vector<std::string> allowed_methods;
};


class Server_config {
public:
	std::string 				root;
	std::vector<std::string>	cgi_extensions;
	std::string 				server_name;
	std::string 				error_pages_path;
	std::vector<int> 			ports;

	std::map<std::string, std::string> execs;
	//        location     path
	std::map<std::string, std::string> locs;

	void add_port(int port) {
		ports.push_back(port);
	}

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

class Server {
public:
	std::vector<Server_config> 	configs;
	std::vector<SocketTCP> 		sockets;
	std::set<int> ports;

	bool configs_intersects(int port, std::string & server_name) {
		for (int i=0; i < configs.size(); i++) {
			Server_config & conf = configs[i];
			for (int j=0; j < conf.ports.size(); j++) {
				if (conf.ports[j] == port && server_name == conf.server_name)
					return true;
			}
		}
		return false;
	}

	Server_config * match_config(std::string name, int port) {
			
	}

	int add_config(Server_config & config) {
		for ( int i=0; i<config.ports.size(); i++ ) {
			int port = config.ports[i];
			if ( configs_intersects(port, config.server_name) )
				return ERROR;
			if ( ports.insert(port).second )
//			if ( ports.find(port) == ports.end() )
				sockets.push_back(SocketTCP(port));
		}
		configs.push_back(config);
		return SUCCESS;
	}

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
};
