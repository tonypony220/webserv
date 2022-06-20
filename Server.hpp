#pragma once
#include "Session.hpp"
#include "SocketTCP.hpp"
#include <algorithm>
#include <map>
#include <vector>
#include <algorithm>
#include <set>

//std::map<std::string, std::string> execs = {std::make_pair(".py", "python3")};
struct Location {
	std::string				 route;
	std::string 			 root;
	std::string 		     redirect_uri;
	bool 					 dir_listing;
	std::vector<std::string> cgi_extensions;
	std::vector<std::string> allowed_methods;

	void allow(std::string) {

	}

	bool method_allowed(std::string & method) {
		return find(allowed_methods.begin(),
			  		allowed_methods.end(),
			        	 method) != allowed_methods.end();
	}
	void enable_cgi(const std::string & file_extension) {
		cgi_extensions.push_back(file_extension);
	}
	bool is_cgi_file_type(std::string & file_type) {
//		log(file_type, " cgi extensions:  ");
//		p(cgi_extensions);
		std::vector<std::string>::iterator found = find(
				cgi_extensions.begin(),
				cgi_extensions.end(),
				file_type);
		if ( found != cgi_extensions.end())
			return true;
		return false;

	}
};

class Server_config {
public:
	std::string 				root;  // required field
//	std::string 				server_name;
	std::vector<std::string> 	server_names;
	std::string 				error_pages_path;
	std::vector<int> 			ports;
	std::vector<Location>		locs;
	size_t						max_size;
	bool						enable_session;

	std::map<std::string, std::string> execs; // ?
	//        location     path
//	std::map<std::string, std::string> locs;
	Location * route_target_path(std::string & target) {
		// location/this/abc -> root/abc
		size_t max = 0;
		size_t idx = 0;
		for (size_t i=0; i<locs.size(); i++){
			if (target.find(locs[i].route) != std::string::npos) {
				if (locs[i].route.size() > max) {
					max = locs[i].route.size();
					idx = i;
				}
			}
		}
		if (max) {
			target.erase(0, locs[idx].size())
			std::string r = locs[idx].root.size() ? locs[idx].root : root;
			target.insert(0, r.begin(), r.end());
			return &locs[idx];
		}
		return nullptr;

	}

	void add_port(int port) {
		ports.push_back(port);
	}
	void clear() {
		root.clear();  // required field
		server_names.clear();
		error_pages_path.clear();
		ports.clear();
		locs.clear();
		max_size = 0;
		enable_session = false;
	}


};
template <class T>
bool intersects(std::vector<T> a, std::vector<T> b ) {
	std::set<T> s;
	s.insert(a.begin(), a.end());
	s.insert(b.begin(), b.end());
	if (s.size() != a.size() + b.size())
		return true
	return false;
}

class Server {
public:
	std::vector<Server_config> 	configs;
	std::vector<SocketTCP> 		sockets;
	std::set<int> 				ports;
	std::string 				app_name;
	std::map<int, std::vector<*Server_config> >  mapping;

	bool configs_intersects(Server_config & new_config) {
		for (int i=0; i < configs.size(); i++) {
			Server_config & conf = configs[i];
			if (intersects(conf.ports, new_config.ports)
			&& (intersects(conf.server_names, new_config.server_names))) {
//			for (int j=0; j < conf.ports.size(); j++) {
//				if (conf.ports[j] == port && server_name == conf.server_name)
					return true;
			}
		}
		return false;
	}

	Server_config * match_config(std::string name, int port) {
		std::vector<*Server_config> configs = mapping[port];
		size_t i=configs.size();
		for (; i >= 0; i--) {
			if (configs[i]->server_name == name)
				return configs[i];
		}
		return configs[i];
	}

	int add_config(Server_config & config) {
		if ( configs_intersects(config) )
			return ERROR;
		configs.push_back(config);
		// to map pointer on objs in this vector
		Server_config * conf = &configs.back();
		for ( int i=0; i<config.ports.size(); i++ ) {
			int port = config.ports[i];
//			if ( configs_intersects(port, config.server_names) )
//				return ERROR;
			if ( ports.insert(port).second )
//			if ( ports.find(port) == ports.end() )
				sockets.push_back(SocketTCP(port));
			mapping[port].push_back(conf);
		}
		return SUCCESS;
	}
	void set_server_name(std::string & name) { app_name = name; }

	Server( std::vector<int> & ports, std::string root_path)
			: root(root_path), app_name("pony server") {
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
