#pragma once
#include "Session.hpp"
#include "SocketTCP.hpp"
//#include "Request.hpp"
#include <algorithm>
#include <map>
#include <vector>
#include <algorithm>
#include <set>
#include <sys/stat.h>


void clear_path(std::string & path) {
	size_t pos = path.find("//");
	if (pos != std::string::npos)
		path.erase(pos, 1);
}

//bool vars_correct(std::vector<std::string> & vars, std::string *refs[]) {
//	std::set<std::string> r(std::begin(refs), std::end(refs));
//	size_t sz(r.size());
//	r.insert(vars.begin(), vars.end());
//	return sz == r.size();
//}
bool vars_correct(std::vector<std::string> & vars,
				  std::string * begin,
				  std::string * end) {
	std::set<std::string> r(begin, end);
	size_t sz(r.size());
	r.insert(vars.begin(), vars.end());
	return sz == r.size();
}
//std::map<std::string, std::string> execs = {std::make_pair(".py", "python3")};
struct Location {
	std::string				 route;
	std::string 			 root;
	std::string 		     redirect_uri;
	bool 					 dir_listing;
	std::vector<std::string> cgi_extensions;
	std::vector<std::string> filenames;
	std::vector<std::string> allowed_methods;

	void display(std::ostream & o) {
		o << "\tLocations: " << "\n";
		o << "\t\troute: " << route << "\n";
		if (root.size())
		o << "\t\troot: " << root << "\n";
		if (redirect_uri.size())
		o << "\t\tredirect: " << redirect_uri << "\n";
		o << "\t\tautoindex: " << dir_listing << "\n";
		if (cgi_extensions.size()) {
			o << "\t\tcgi's: ";
			p(cgi_extensions);
			o << "\n";
		}
		o << "\t\tmethods: ";
		p(allowed_methods);
		o << "\n";
	}
	void validate(std::stringstream & err) {
		if (root.size() && !valid_dir_path(root)) {
			err << "bad root: " << root << " " << strerror(errno);
//			return;
		}
		if (!vars_correct(allowed_methods,
						  std::begin(HttpMethodsImplemented),
						  std::end(HttpMethodsImplemented))) {
			err << "bad methods "<< "\n";
//			return;
		}
		if (!vars_correct(cgi_extensions,
						  std::begin(cgi_extensions_supported),
						  std::end(cgi_extensions_supported))) {
			err << "bad cgi: " << "\n";
//			return;
		}
	}

	bool method_allowed(std::string & method) {
		if (allowed_methods.empty())
			return true;
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
	void clear() {
		route.clear();
		root.clear();
		redirect_uri.clear();
		dir_listing = false;
		cgi_extensions.clear();
		allowed_methods.clear();
	}
};
std::ostream & operator<<( std::ostream & o, Location & s ) {
	s.display(o);
	return ( o );
}

class Server_config {
public:
	std::string 				root;  // required field
	std::vector<std::string> 	server_names;
	std::string 				error_pages_path;
	std::vector<int> 			ports;
	std::vector<Location>		locs;
	size_t						max_size;
	bool						enable_session;

	void display(std::ostream & o) {
		o << "Server Config: " << "\n";
		o << "\troot: " << root << "\n";
		o << "\terror pages path: " << root << "\n";
		o << "\tmax size: " << max_size << "\n";
		o << "\tenable sessions: " << enable_session << "\n";
		o << "\tserver names: ";
		p(server_names);
		o << "\n";
		o << "\tports: ";
		p(ports);
		o << "\n";
		o << "\tLocations:";
		p(locs);
	}

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
			target.erase(0, locs[idx].route.size());
			std::string r = locs[idx].root.size() ? locs[idx].root : root;
			target.insert(target.begin(), r.begin(), r.end());
			return &locs[idx];
		}
		return nullptr;

	}
	bool server_name_in_config_names(std::string & name) {
		return easyfind(server_names, name);
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

std::ostream & operator<<( std::ostream & o, Server_config & s ) {
	s.display(o);
	return ( o );
}


template <class T>
bool intersects(std::vector<T> a, std::vector<T> b ) {
	std::set<T> s;
	s.insert(a.begin(), a.end());
	s.insert(b.begin(), b.end());
	if (s.size() != a.size() + b.size())
		return true;
	return false;
}

class Server {
public:
	std::vector<Server_config> 					 configs;
	std::vector<SocketTCP> 						 sockets;
	std::set<int> 								 ports;
	std::string 								 app_name;
	std::map<int, std::vector<Server_config*> >  mapping;

	std::stringstream err;

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

	void display_configs_addresses() { // debuging
		for (int i = 0; i < configs.size(); i++)
			std::cout << "config* = " << &configs[i] << "\n";

		std::map<int, std::vector<Server_config*> >::iterator it = mapping.begin();
		for (; it != mapping.end(); it++) {
			for (int i = 0; i < it->second.size(); i++)
				std::cout << "port config = " << it->second[i] << "\n";
		}

	}

	Server_config * get_default_config() { return &configs[0]; }

	Server_config * match_config(std::string name, int port) {
//		display_configs_addresses();
		if (!port)
			return &configs[0];
		std::vector<Server_config*> conf = mapping[port];
		int i = conf.size();
//		if (!i)
//			std::cerr << "<<<<<<<<< no config?" << conf[i] << "\n";
		i--;
		for (; i > 0; i--) {
			if (conf[i]->server_name_in_config_names(name))
				return conf[i];
		}
		std::cout << "<<<<<<<<< saved config" << conf[i] << "\n";
		return conf[i]; // default
	}

	void validate_config(Server_config & config) {
		if (ok() && config.root.empty())
			err << "root required";
		if (ok() && !valid_dir_path(config.root))
			err << "bad root: " << config.root << " " << strerror(errno);
		if (ok()
		&& config.error_pages_path.size()
		&& !valid_dir_path(config.error_pages_path))
			err << "bad err pages path: " << config.root << " "
			<< strerror(errno);
		for (int i = 0; i < config.locs.size() && ok(); i++) {
			config.locs[i].validate(err);
		}
	}

	bool ok() { return !err.rdbuf()->in_avail(); }

	int error() {
		std::cerr << RED << "config error: " << err.rdbuf() << RESET"\n";
		return ERROR;
	}

	int add_config(Server_config & config) {
		validate_config(config);
		if (!ok())
			return error();
		if ( configs_intersects(config) ) {
//			err << RED"config error: server names conflict\n"RESET;
			return error();
		}
		configs.push_back(config);
		// to map pointer on objs in this vector
		Server_config * conf = &configs.back();
//		std::cout << "<<<<<<<< parser config" << &config << "\n";
//		std::cout << "<<<<<<<< serv   config" << conf << "\n";
		for ( int i=0; i<config.ports.size(); i++ ) {
			int port = config.ports[i];
			if ( ports.insert(port).second )
//			if ( ports.find(port) == ports.end() )
				sockets.push_back( SocketTCP(port) );
			mapping[port].push_back(conf);
		}
		return SUCCESS;
	}

	int create(std::vector<Server_config> & parser_configs) {
		configs.reserve(parser_configs.size());
		for ( size_t i = 0; i < parser_configs.size(); i++ ) {
			if (add_config(parser_configs[i])) {
				return EXIT_FAILURE;
			}
		}
		return EXIT_SUCCESS;
	}
	std::string generate_session_id() {
		return "1";
	}

	void set_server_name(std::string & name) { app_name = name; }

	Server() :app_name("pony server") {}
	~Server() {}
	//Server( const Server & copy ) { *this = copy; }
	//Server & operator=( const Server & other ) {}
};
